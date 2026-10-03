#include <adwaita.h>
#include <node.h>

GtkWidget *node_render(node_t *node) {
  switch (node->type) {
  case node_type_text: {
    return gtk_label_new_with_mnemonic((char *)node->data);
  }
  case node_type_box: {
    GtkWidget *box =
        gtk_box_new(*(node_type_box_orientation_type_t *)node->data ==
                            node_type_box_orientation_type_vertical
                        ? GTK_ORIENTATION_VERTICAL
                        : GTK_ORIENTATION_HORIZONTAL,
                    0);

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
    GtkWidget *button = gtk_button_new();

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
  g_autoptr(GVariant) child = NULL;

  // TODO: use ReadOne instead
  ret = g_dbus_proxy_call_sync(
      proxy, "Read",
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

  g_variant_get(ret, "(v)", &child);
  g_variant_get(child, "v", out);

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
  GtkWidget *window = gtk_application_window_new(app);
  GtkSettings *settings = gtk_settings_get_default();
  char *desktop = getenv("XDG_CURRENT_DESKTOP");
  node_t *node = user_data;

  if (!(desktop && strstr(desktop, "GNOME"))) {
    init_portal(settings);
  }

  gtk_window_set_title(GTK_WINDOW(window), "Hello");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 250);

  for (size_t i = 0; i < node->children->size; i++) {
    node_t *child = node->children->items[i];
    GtkWidget *widget = node_render(child);

    if (widget) {
      gtk_window_set_child(GTK_WINDOW(window), widget);
    }
  }

  gtk_window_present(GTK_WINDOW(window));
}

int node_run(node_t *node) {
  const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
  int status;
  void *app;

  if (desktop && strstr(desktop, "GNOME")) {
    app = adw_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  } else {
    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  }

  g_signal_connect(app, "activate", G_CALLBACK(activate), node);
  status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

  return status;
}
