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

static void activate(GtkApplication *app, gpointer user_data) {
  node_t *node = user_data;
  GtkWidget *window = gtk_application_window_new(app);
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
