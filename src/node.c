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

  if (node->type == node_type_button) {
    free(node->data);
  }

  free(node->children->items);
  free(node->children);
  free(node);
}
