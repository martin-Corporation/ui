#include <node.h>

int main(int argc, char **argv) {
  node_t *window = node_initialize(node_type_window, "com.mrtn.demo");
  node_type_box_orientation_type_t orientation =
      node_type_box_orientation_type_vertical;

  node_t *box = node_initialize(node_type_box, &orientation);
  node_type_button_variant_type_t variant =
      node_type_button_variant_type_primary;

  node_t *text = node_initialize(node_type_text, "Welcome to martinUI!");
  node_t *button = node_type_button_initialize("Button", &variant);

  list_append(window->children, box);
  list_append(box->children, text);
  list_append(box->children, button);
  int status = node_run(window, argc, argv);
  node_destroy(window);

  return status;
}
