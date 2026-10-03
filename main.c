#include <node.h>

// Backend-provided function.
int node_run(node_t *node);

int main(void) {
  node_t *window = node_initialize(node_type_window, NULL);
  node_type_box_orientation_type_t orientation =
      node_type_box_orientation_type_vertical;

  node_t *box = node_initialize(node_type_box, &orientation);
  node_t *button = node_initialize(node_type_button, NULL);
  node_t *text = node_initialize(node_type_text, "Button");

  list_append(window->children, box);
  list_append(box->children, button);
  list_append(button->children, text);
  node_run(window);
  node_destroy(window);

  return 0;
}
