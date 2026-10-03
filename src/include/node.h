#pragma once
#include <list.h>

typedef enum node_type {
  node_type_window,
  node_type_text,
  node_type_button
} node_type_t;

typedef struct node {
  node_type_t type;
  void *data;
  list_t *children;
} node_t;

node_t *node_initialize(node_type_t type, void *data);
void node_destroy(node_t *node);
