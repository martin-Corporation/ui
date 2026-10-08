#include <adwaita.h>
#include <node.h>

GtkWidget *node_render(node_t *node) {
  switch (node->type) {
    case node_type_text: {
      return gtk_label_new_with_mnemonic((char *)node->data);
    }
    case node_type_alert_dialog: {
      node_type_alert_dialog_data_t *data = node->data;
      GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", data->title);
      data->rendered = dialog;

      if (data->description) {
        gtk_alert_dialog_set_detail(dialog, data->description);
      }

      break;
    }
    case node_type_box: {
      node_type_box_data_t *data = node->data;
      GtkWidget *box =
          gtk_box_new((GtkOrientation)data->orientation, data->spacing);

      for (size_t i = 0; i < node->children->size; i++) {
        node_t *child = node->children->items[i];
        GtkWidget *widget = node_render(child);

        if (widget) {
          gtk_box_append(GTK_BOX(box), widget);
        }
      }

      return box;
    }
    case node_type_button: {
      node_type_button_data_t *data = node->data;
      GtkWidget *button = gtk_button_new();

      if (data->clicked) {
        g_signal_connect(button, "clicked", G_CALLBACK(data->clicked), node);
      }

      switch (data->variant) {
        case node_type_button_variant_type_primary: {
          gtk_widget_add_css_class(button, "suggested-action");
          break;
        }
        case node_type_button_variant_type_destructive: {
          gtk_widget_add_css_class(button, "destructive-action");
          break;
        }
        default: {
          break;
        }
      }

      for (size_t i = 0; i < node->children->size; i++) {
        node_t *child = node->children->items[i];
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

typedef enum color_scheme {
  color_scheme_default,
  color_scheme_prefer_dark,
  color_scheme_prefer_light
} color_scheme_t;

void set_color_scheme(GtkSettings *settings, GVariant *variant) {
  color_scheme_t color_scheme = g_variant_get_uint32(variant);

  if (color_scheme > color_scheme_prefer_light) {
    color_scheme = color_scheme_default;
  }

  g_object_set(settings, "gtk-application-prefer-dark-theme",
               color_scheme == color_scheme_prefer_dark, NULL);
}

void settings_portal_changed_cb(GDBusProxy *proxy, const char *sender_name,
                                const char *signal_name, GVariant *parameters,
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
  GtkWidget *window = gtk_application_window_new(app);
  GtkSettings *settings = gtk_settings_get_default();
  node_t *node = user_data;
  node_type_window_data_t *data = node->data;

  if (!(desktop && strstr(desktop, "GNOME"))) {
    init_portal(settings);
  }

  gtk_window_set_default_size(GTK_WINDOW(window), data->width, data->height);

  for (size_t i = 0; i < node->children->size; i++) {
    node_t *child = node->children->items[i];
    GtkWidget *widget = node_render(child);

    if (widget) {
      gtk_window_set_child(GTK_WINDOW(window), widget);
    }
  }

  gtk_window_present(GTK_WINDOW(window));
}

void node_type_alert_dialog_show(node_t *node) {
  node_type_alert_dialog_data_t *data = node->data;
  GtkAlertDialog *dialog = data->rendered;
  gtk_alert_dialog_show(dialog, NULL);
}

int node_type_window_run(node_t *node, int argc, char **argv) {
  const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
  node_type_window_data_t *data = node->data;
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
