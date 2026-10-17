#include <node.h>

Node *dialog;
Node *label;

void clicked(void) {
  node_type_text_update(label, "Hi!");
  node_type_alert_dialog_show(dialog);
}

int main(int argc, char **argv) {
  Node *window = node_type_window_initialize("com.mrtn.demo", 500, 250);
  dialog = node_type_alert_dialog_initialize(
      "Hello, World!", "This is an example alert dialog.", NULL);

  Node *box =
      node_type_box_initialize(NodeTypeBoxOrientationTypeVertical, true, true);

  Node *text = node_type_text_initialize("Welcome to martinUI!", true);
  Node *button = node_type_button_initialize(
      "Button", NodeTypeButtonVariantTypePrimary, clicked);

  label = button->children->items[0];
  list_append(window->children, dialog);
  list_append(window->children, box);
  list_append(box->children, text);
  list_append(box->children, button);
  int status = node_type_window_run(window, argc, argv);
  node_destroy(window);

  return status;
}
