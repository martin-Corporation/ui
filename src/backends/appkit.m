#import <AppKit/AppKit.h>
#include <node.h>
#import <objc/runtime.h>

static NSWindow *window;

@interface NodeTypeButtonEventHandler : NSObject
@property(nonatomic, assign) Node *node;
@end
@implementation NodeTypeButtonEventHandler

- (void)handleEvent:(id)sender {
  (void)sender;

  if (self.node) {
    Node *node = self.node;
    NodeTypeButtonData *data = node->data;

    if (data->clicked) {
      data->clicked();
    }
  }
}

@end

static void node_type_button_connect_event(NSButton *button, Node *node) {
  NodeTypeButtonEventHandler *listener = NodeTypeButtonEventHandler.new;

  listener.node = node;
  button.target = listener;
  button.action = @selector(handleEvent:);
  objc_setAssociatedObject(button, node, listener,
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

static NSView *node_render(Node *node) {
  switch (node->type) {
    case NodeTypeWindow: {
      break;
    }
    case NodeTypeAlertDialog: {
      NodeTypeAlertDialogData *data = node->data;
      NSAlert *alert = NSAlert.new;
      data->rendered = alert;
      alert.messageText = [NSString stringWithUTF8String:data->title];

      if (data->description) {
        alert.informativeText =
            [NSString stringWithUTF8String:data->description];
      }

      break;
    }
    case NodeTypeBox: {
      NSStackView *stack = [NSStackView.alloc initWithFrame:NSZeroRect];
      stack.translatesAutoresizingMaskIntoConstraints = NO;
      NodeTypeBoxData *data = node->data;
      stack.spacing = data->spaced ? 8 : 0;
      stack.orientation =
          data->orientation == NodeTypeBoxOrientationTypeHorizontal
              ? NSUserInterfaceLayoutOrientationHorizontal
              : NSUserInterfaceLayoutOrientationVertical;

      if (data->orientation == NodeTypeBoxOrientationTypeVertical) {
        stack.alignment = NSLayoutAttributeLeading;
      }

      if (data->padded) {
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20);
      }

      List *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        Node *child = children->items[i];
        NSView *view = node_render(child);

        if (view) {
          [stack addView:view
               inGravity:data->orientation == NodeTypeBoxOrientationTypeVertical
                             ? NSStackViewGravityTop
                             : NSStackViewGravityLeading];
        }
      }

      return stack;
    }
    case NodeTypeButton: {
      NSButton *button = [NSButton buttonWithTitle:@"" target:nil action:nil];
      NodeTypeButtonData *data = node->data;

      if (data->clicked) {
        node_type_button_connect_event(button, node);
      }

      if (@available(macOS 26.0, *)) {
        button.bezelStyle = NSBezelStyleGlass;

        switch (data->variant) {
          case NodeTypeButtonVariantTypePrimary: {
            button.tintProminence = NSTintProminencePrimary;
            break;
          }
          case NodeTypeButtonVariantTypeSecondary: {
            button.tintProminence = NSTintProminenceNone;
            break;
          }
          case NodeTypeButtonVariantTypeDestructive: {
            button.hasDestructiveAction = YES;
            break;
          }
          default: {
            break;
          }
        }
      }

      List *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        Node *child = children->items[i];

        if (child->type == NodeTypeText) {
          NodeTypeTextData *data = child->data;
          data->rendered = button;
          button.title = [NSString stringWithUTF8String:data->text];
        }
      }

      return button;
    }
    case NodeTypeText: {
      NodeTypeTextData *data = node->data;
      NSString *string = [NSString stringWithUTF8String:data->text];
      NSTextField *field = [NSTextField labelWithString:string];
      field.selectable = data->selectable;
      data->rendered = field;

      return field;
    }
  }

  return nil;
}

void node_type_alert_dialog_show(Node *node) {
  NodeTypeAlertDialogData *data = node->data;
  NSAlert *alert = data->rendered;
  [alert beginSheetModalForWindow:window
                completionHandler:(data->handler == nil ? nil
                                                        : ^(NSModalResponse _) {
                                                            data->handler();
                                                          })];
}

void node_type_text_update(Node *node, char *text) {
  NodeTypeTextData *data = node->data;
  data->text = text;

  if (data->rendered) {
    id rendered = data->rendered;
    NSString *string = [NSString stringWithUTF8String:data->text];

    if ([rendered isKindOfClass:NSTextField.class]) {
      NSTextField *field = rendered;
      field.stringValue = string;
    } else if ([rendered isKindOfClass:NSButton.class]) {
      NSButton *button = rendered;
      button.title = string;
    }
  }
}

int node_type_window_run(Node *node, int argc, char **argv) {
  (void)argc;
  (void)argv;

  NSString *bundleName =
      [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];

  NSString *name =
      bundleName ? bundleName : NSProcessInfo.processInfo.processName;

  NSApplication *app = NSApplication.sharedApplication;
  app.activationPolicy = NSApplicationActivationPolicyRegular;

  NodeTypeWindowData *data = node->data;
  window = [NSWindow.alloc
      initWithContentRect:NSMakeRect(0, 0, data->width, data->height)
                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                           NSWindowStyleMaskMiniaturizable |
                           NSWindowStyleMaskResizable)
                  backing:NSBackingStoreBuffered
                    defer:NO];

  [window center];
  window.title = name;
  NSView *root = window.contentView;
  List *children = node->children;

  for (size_t i = 0; i < children->size; i++) {
    Node *child = children->items[i];
    NSView *view = node_render(child);

    if (view) {
      [root addSubview:view];
      [NSLayoutConstraint activateConstraints:@[
        [view.leftAnchor constraintEqualToAnchor:root.leftAnchor],
        [view.topAnchor constraintEqualToAnchor:root.topAnchor],
        [view.bottomAnchor constraintEqualToAnchor:root.bottomAnchor],
        [view.rightAnchor constraintEqualToAnchor:root.rightAnchor]
      ]];
    }
  }

  [window makeKeyAndOrderFront:nil];
  [app run];

  return 0;
}
