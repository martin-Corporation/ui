#include <node.h>

// Backend-provided function.
int node_run(node_t *node);

int main(void) {
  node_t *window = node_initialize(node_type_window, NULL);
  node_t *button = node_initialize(node_type_button, NULL);
  node_t *text = node_initialize(node_type_text, "Button");

  list_append(button->children, text);
  list_append(window->children, button);
  node_run(window);
  node_destroy(window);

  return 0;
}
