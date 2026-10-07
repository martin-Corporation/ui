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

typedef struct node_type_box_data {
  size_t spacing;
  node_type_box_orientation_type_t orientation;
} node_type_box_data_t;

typedef enum node_type_button_variant_type {
  node_type_button_variant_type_primary,
  node_type_button_variant_type_secondary,
  node_type_button_variant_type_destructive
} node_type_button_variant_type_t;

typedef struct node_type_button_data {
  void (*clicked)(void);
  node_type_button_variant_type_t variant;
} node_type_button_data_t;

typedef struct node {
  node_type_t type;
  void *data;
  list_t *children;
} node_t;

#if defined(__cplusplus)
extern "C" {
#endif
node_t *node_type_button_initialize(char *label,
                                    node_type_button_variant_type_t variant,
                                    void (*clicked)(void));
node_t *node_type_box_initialize(node_type_box_orientation_type_t orientation,
                                 size_t spacing);
int node_type_window_run(node_t *node, int argc, char **argv);
node_t *node_initialize(node_type_t type, void *data);
void node_destroy(node_t *node);
#if defined(__cplusplus)
}
#endif
