#include <node.h>
#include <stdlib.h>

Node *node_type_button_initialize(char *label,
                                  NodeTypeButtonVariantType variant,
                                  NodeEventHandler clicked) {
  NodeTypeButtonData *data = malloc(sizeof(NodeTypeButtonData));
  Node *button = node_initialize(NodeTypeButton, data);
  Node *text = node_type_text_initialize(label, false);
  data->variant = variant;
  data->clicked = clicked;
  list_append(button->children, text);

  return button;
}

Node *node_type_box_initialize(NodeTypeBoxOrientationType orientation,
                               bool padded, bool spaced) {
  NodeTypeBoxData *data = malloc(sizeof(NodeTypeBoxData));
  Node *box = node_initialize(NodeTypeBox, data);
  data->orientation = orientation;
  data->spaced = spaced;
  data->padded = padded;

  return box;
}

Node *node_type_alert_dialog_initialize(char *title, char *description,
                                        NodeEventHandler handler) {
  NodeTypeAlertDialogData *data = malloc(sizeof(NodeTypeAlertDialogData));
  Node *dialog = node_initialize(NodeTypeAlertDialog, data);
  data->description = description;
  data->handler = handler;
  data->rendered = NULL;
  data->title = title;

  return dialog;
}

Node *node_type_window_initialize(char *id, size_t width, size_t height) {
  NodeTypeWindowData *data = malloc(sizeof(NodeTypeWindowData));
  Node *window = node_initialize(NodeTypeWindow, data);
  data->height = height;
  data->width = width;
  data->id = id;

  return window;
}

Node *node_type_text_initialize(char *text, bool selectable) {
  NodeTypeTextData *data = malloc(sizeof(NodeTypeTextData));
  Node *node = node_initialize(NodeTypeText, data);
  data->selectable = selectable;
  data->rendered = NULL;
  data->text = text;

  return node;
}

Node *node_initialize(NodeType type, void *data) {
  Node *node = malloc(sizeof(Node));
  node->type = type;
  node->data = data;
  node->children = list_initialize(1, sizeof(Node *));

  return node;
}

void node_destroy(Node *node) {
  for (size_t i = 0; i < node->children->size; i++) {
    Node *child = node->children->items[i];
    node_destroy(child);
  }

  if (node->type == NodeTypeWindow || node->type == NodeTypeAlertDialog ||
      node->type == NodeTypeBox || node->type == NodeTypeButton ||
      node->type == NodeTypeText) {
    free(node->data);
  }

  free(node->children->items);
  free(node->children);
  free(node);
}
