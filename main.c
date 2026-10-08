#include <node.h>
#include <stdio.h>

node_t *dialog;
void clicked(void) { node_type_alert_dialog_show(dialog); }

int main(int argc, char **argv) {
  node_t *window = node_type_window_initialize("com.mrtn.demo", 500, 250);
  dialog = node_type_alert_dialog_initialize(
      "Hello, World!", "This is an example alert dialog.");

  node_t *box =
      node_type_box_initialize(node_type_box_orientation_type_vertical, 0);

  node_t *text = node_initialize(node_type_text, "Welcome to martinUI!");
  node_t *button = node_type_button_initialize(
      "Button", node_type_button_variant_type_primary, clicked);

  list_append(window->children, dialog);
  list_append(window->children, box);
  list_append(box->children, text);
  list_append(box->children, button);
  int status = node_type_window_run(window, argc, argv);
  node_destroy(window);

  return status;
}
