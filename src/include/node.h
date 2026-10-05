#pragma once
#include <list.h>

typedef enum node_type {
  node_type_window,
  node_type_box,
  node_type_button,
  node_type_text
} node_type_t;

typedef enum node_type_box_orientation_type {
  node_type_box_orientation_type_horizontal,
  node_type_box_orientation_type_vertical
} node_type_box_orientation_type_t;

typedef enum node_type_button_variant_type {
  node_type_button_variant_type_primary,
  node_type_button_variant_type_secondary,
  node_type_button_variant_type_destructive
} node_type_button_variant_type_t;

typedef struct node {
  node_type_t type;
  void *data;
  list_t *children;
} node_t;

node_t *node_initialize(node_type_t type, void *data);
int node_run(node_t *node, int argc, char **argv);
void node_destroy(node_t *node);
