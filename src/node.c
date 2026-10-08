#include <node.h>
#include <stdlib.h>

node_t *node_type_button_initialize(char *label,
                                    node_type_button_variant_type_t variant,
                                    void (*clicked)(void)) {
  node_type_button_data_t *data = malloc(sizeof(node_type_button_data_t));
  node_t *button = node_initialize(node_type_button, data);
  node_t *text = node_initialize(node_type_text, label);
  data->variant = variant;
  data->clicked = clicked;
  list_append(button->children, text);

  return button;
}

node_t *node_type_box_initialize(node_type_box_orientation_type_t orientation,
                                 size_t spacing) {
  node_type_box_data_t *data = malloc(sizeof(node_type_box_data_t));
  node_t *box = node_initialize(node_type_box, data);
  data->orientation = orientation;
  data->spacing = spacing;

  return box;
}

node_t *node_type_alert_dialog_initialize(char *title, char *description) {
  node_type_alert_dialog_data_t *data =
      malloc(sizeof(node_type_alert_dialog_data_t));

  node_t *dialog = node_initialize(node_type_alert_dialog, data);
  data->description = description;
  data->rendered = NULL;
  data->title = title;

  return dialog;
}

node_t *node_type_window_initialize(char *id, size_t width, size_t height) {
  node_type_window_data_t *data = malloc(sizeof(node_type_window_data_t));
  node_t *window = node_initialize(node_type_window, data);
  data->height = height;
  data->width = width;
  data->id = id;

  return window;
}

node_t *node_initialize(node_type_t type, void *data) {
  node_t *node = malloc(sizeof(node_t));
  node->type = type;
  node->data = data;
  node->children = list_initialize(1, sizeof(node_t *));

  return node;
}

void node_destroy(node_t *node) {
  for (size_t i = 0; i < node->children->size; i++) {
    node_t *child = node->children->items[i];
    node_destroy(child);
  }

  if (node->type == node_type_box || node->type == node_type_button) {
    free(node->data);
  }

  free(node->children->items);
  free(node->children);
  free(node);
}
