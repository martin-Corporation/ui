#include <adwaita.h>
#include <node.h>

static GtkWidget *window;

static GtkWidget *node_render(Node *node) {
  switch (node->type) {
    case NodeTypeText: {
      NodeTypeTextData *data = node->data;
      GtkWidget *label = gtk_label_new(data->text);
      gtk_label_set_selectable(GTK_LABEL(label), data->selectable);
      data->rendered = label;

      return label;
    }
    case NodeTypeAlertDialog: {
      NodeTypeAlertDialogData *data = node->data;
      GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", data->title);
      data->rendered = dialog;

      if (data->description) {
        gtk_alert_dialog_set_detail(dialog, data->description);
      }

      break;
    }
    case NodeTypeBox: {
      NodeTypeBoxData *data = node->data;
      size_t padding = data->padded ? 12 : 0;
      GtkWidget *box = gtk_box_new(
          (GtkOrientation)data->orientation,
          data->spaced
              ? (data->orientation == NodeTypeBoxOrientationTypeHorizontal ? 6
                                                                           : 12)
              : 0);

      gtk_widget_set_margin_start(box, padding);
      gtk_widget_set_margin_end(box, padding);
      gtk_widget_set_margin_top(box, padding);
      gtk_widget_set_margin_bottom(box, padding);

      List *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        Node *child = children->items[i];
        GtkWidget *widget = node_render(child);

        if (widget) {
          gtk_box_append(GTK_BOX(box), widget);
        }
      }

      return box;
    }
    case NodeTypeButton: {
      NodeTypeButtonData *data = node->data;
      GtkWidget *button = gtk_button_new();

      if (data->clicked) {
        g_signal_connect(button, "clicked", G_CALLBACK(data->clicked), node);
      }

      switch (data->variant) {
        case NodeTypeButtonVariantTypePrimary: {
          gtk_widget_add_css_class(button, "suggested-action");
          break;
        }
        case NodeTypeButtonVariantTypeDestructive: {
          gtk_widget_add_css_class(button, "destructive-action");
          break;
        }
        default: {
          break;
        }
      }

      List *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        Node *child = children->items[i];
        GtkWidget *widget = node_render(child);

        if (widget) {
          gtk_button_set_child(GTK_BUTTON(button), widget);
        }
      }

      return button;
    }
    default: {
      break;
    }
  }

  return NULL;
}

typedef enum ColorScheme {
  ColorSchemeDefault,
  ColorSchemePreferDark,
  ColorSchemePreferLight
} ColorScheme;

static void set_color_scheme(GtkSettings *settings, GVariant *variant) {
  ColorScheme color_scheme = g_variant_get_uint32(variant);

  if (color_scheme > ColorSchemePreferLight) {
    color_scheme = ColorSchemeDefault;
  }

  g_object_set(settings, "gtk-application-prefer-dark-theme",
               color_scheme == ColorSchemePreferDark, NULL);
}

static void settings_portal_changed_cb(GDBusProxy *proxy,
                                       const char *sender_name,
                                       const char *signal_name,
                                       GVariant *parameters,
                                       GtkSettings *settings) {
  (void)proxy;
  (void)sender_name;

  const char *namespace;
  const char *name;
  g_autoptr(GVariant) value = NULL;

  if (g_strcmp0(signal_name, "SettingChanged")) {
    return;
  }

  g_variant_get(parameters, "(&s&sv)", &namespace, &name, &value);

  if (g_strcmp0(namespace, "org.freedesktop.appearance") ||
      g_strcmp0(name, "color-scheme")) {
    return;
  }

  set_color_scheme(settings, value);
}

static gboolean read_color_scheme(GDBusProxy *proxy, GVariant **out) {
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) ret = NULL;
  ret = g_dbus_proxy_call_sync(
      proxy, "ReadOne",
      g_variant_new("(ss)", "org.freedesktop.appearance", "color-scheme"),
      G_DBUS_CALL_FLAGS_NONE, G_MAXINT, NULL, &error);

  if (error) {
    if (error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_SERVICE_UNKNOWN) {
      g_debug("Portal not found: %s", error->message);
      return FALSE;
    }

    if (error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_UNKNOWN_METHOD) {
      g_debug("Portal doesn't provide settings: %s", error->message);
      return FALSE;
    }

    g_critical("Couldn't read the color-scheme setting: %s", error->message);
    return FALSE;
  }

  g_variant_get(ret, "(v)", out);
  return TRUE;
}

static void init_portal(GtkSettings *settings) {
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) value = NULL;
  GDBusProxy *settings_portal = g_dbus_proxy_new_for_bus_sync(
      G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
      "org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
      "org.freedesktop.portal.Settings", NULL, &error);

  if (error) {
    g_debug("Settings portal not found: %s", error->message);
    return;
  }

  if (!read_color_scheme(settings_portal, &value)) {
    return;
  }

  set_color_scheme(settings, value);
  g_signal_connect(settings_portal, "g-signal",
                   G_CALLBACK(settings_portal_changed_cb), settings);
}

static void activate(GtkApplication *app, gpointer user_data) {
  const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
  GtkSettings *settings = gtk_settings_get_default();
  window = gtk_application_window_new(app);
  Node *node = user_data;
  NodeTypeWindowData *data = node->data;

  if (!(desktop && strstr(desktop, "GNOME"))) {
    init_portal(settings);
  }

  gtk_window_set_default_size(GTK_WINDOW(window), data->width, data->height);

  List *children = node->children;

  for (size_t i = 0; i < children->size; i++) {
    Node *child = children->items[i];
    GtkWidget *widget = node_render(child);

    if (widget) {
      gtk_window_set_child(GTK_WINDOW(window), widget);
    }
  }

  gtk_window_present(GTK_WINDOW(window));
}

static void node_type_alert_dialog_handle_event(GObject *source,
                                                GAsyncResult *result,
                                                gpointer user_data) {
  NodeTypeAlertDialogData *data = user_data;
  gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, NULL);

  if (data->handler != NULL) {
    data->handler();
  }
}

void node_type_alert_dialog_show(Node *node) {
  NodeTypeAlertDialogData *data = node->data;
  GtkAlertDialog *dialog = data->rendered;
  gtk_alert_dialog_choose(
      dialog, GTK_WINDOW(window), NULL,
      data->handler != NULL ? node_type_alert_dialog_handle_event : NULL, data);
}

void node_type_text_update(Node *node, char *text) {
  NodeTypeTextData *data = node->data;
  data->text = text;

  if (data->rendered) {
    GtkWidget *label = data->rendered;
    gtk_label_set_text(GTK_LABEL(label), data->text);
  }
}

int node_type_window_run(Node *node, int argc, char **argv) {
  const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
  NodeTypeWindowData *data = node->data;
  int status;
  void *app;

  if (desktop && strstr(desktop, "GNOME")) {
    app = adw_application_new(data->id, G_APPLICATION_DEFAULT_FLAGS);
  } else {
    app = gtk_application_new(data->id, G_APPLICATION_DEFAULT_FLAGS);
  }

  g_signal_connect(app, "activate", G_CALLBACK(activate), node);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
