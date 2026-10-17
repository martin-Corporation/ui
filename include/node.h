#pragma once
#include <list.h>
#include <stdbool.h>

typedef enum NodeType {
  NodeTypeWindow,
  NodeTypeAlertDialog,
  NodeTypeBox,
  NodeTypeButton,
  NodeTypeText
} NodeType;

typedef struct NodeTypeWindowData {
  char *id;
  size_t width;
  size_t height;
} NodeTypeWindowData;

typedef void (*NodeEventHandler)(void);

typedef struct NodeTypeAlertDialogData {
  char *title;
  char *description;
  void *rendered;
  NodeEventHandler handler;
} NodeTypeAlertDialogData;

typedef enum NodeTypeBoxOrientationType {
  NodeTypeBoxOrientationTypeHorizontal,
  NodeTypeBoxOrientationTypeVertical
} NodeTypeBoxOrientationType;

typedef struct NodeTypeBoxData {
  bool padded;
  bool spaced;
  NodeTypeBoxOrientationType orientation;
} NodeTypeBoxData;

typedef enum NodeTypeButtonVariantType {
  NodeTypeButtonVariantTypePrimary,
  NodeTypeButtonVariantTypeSecondary,
  NodeTypeButtonVariantTypeDestructive
} NodeTypeButtonVariantType;

typedef struct NodeTypeButtonData {
  NodeEventHandler clicked;
  NodeTypeButtonVariantType variant;
} NodeTypeButtonData;

typedef struct NodeTypeTextData {
  char *text;
  void *rendered;
  bool selectable;
} NodeTypeTextData;

typedef struct Node {
  NodeType type;
  void *data;
  List *children;
} Node;

#if defined(__cplusplus)
extern "C" {
#endif
Node *node_type_button_initialize(char *label,
                                  NodeTypeButtonVariantType variant,
                                  NodeEventHandler clicked);
Node *node_type_alert_dialog_initialize(char *title, char *description,
                                        NodeEventHandler handler);
Node *node_type_box_initialize(NodeTypeBoxOrientationType orientation,
                               bool padded, bool spaced);
Node *node_type_window_initialize(char *id, size_t width, size_t height);
Node *node_type_text_initialize(char *text, bool selectable);
int node_type_window_run(Node *node, int argc, char **argv);
void node_type_text_update(Node *node, char *text);
Node *node_initialize(NodeType type, void *data);
void node_type_alert_dialog_show(Node *node);
void node_destroy(Node *node);
#if defined(__cplusplus)
}
#endif
