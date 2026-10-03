#include <node.h>
#include <stdlib.h>

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

  free(node->children->items);
  free(node->children);
  free(node);
}
