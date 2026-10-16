#import <AppKit/AppKit.h>
#include <node.h>
#import <objc/runtime.h>

@interface node_event_listener_t : NSObject
@property(nonatomic, assign) node_t *node;
@end
@implementation node_event_listener_t

- (void)handleEvent:(id)sender {
  (void)sender;

  if (self.node) {
    node_t *node = self.node;
    node_type_button_data_t *data = node->data;

    if (data->clicked) {
      data->clicked();
    }
  }
}

@end

static void node_connect_event(NSButton *button, node_t *node) {
  node_event_listener_t *listener = node_event_listener_t.new;
  listener.node = node;
  button.target = listener;
  button.action = @selector(handleEvent:);
  objc_setAssociatedObject(button, node, listener,
                           OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

static NSView *node_render(node_t *node) {
  switch (node->type) {
    case node_type_window: {
      break;
    }
    case node_type_alert_dialog: {
      node_type_alert_dialog_data_t *data = node->data;
      NSAlert *alert = NSAlert.new;
      data->rendered = alert;
      alert.messageText = [NSString stringWithUTF8String:data->title];

      if (data->description) {
        alert.informativeText =
            [NSString stringWithUTF8String:data->description];
      }

      break;
    }
    case node_type_box: {
      NSStackView *stack = [NSStackView stackViewWithViews:@[]];
      node_type_box_data_t *data = node->data;
      stack.spacing = data->spaced ? 8 : 0;
      stack.orientation =
          data->orientation == node_type_box_orientation_type_horizontal
              ? NSUserInterfaceLayoutOrientationHorizontal
              : NSUserInterfaceLayoutOrientationVertical;

      if (data->orientation == node_type_box_orientation_type_vertical) {
        stack.alignment = NSLayoutAttributeLeading;
      }

      if (data->padded) {
        stack.edgeInsets = NSEdgeInsetsMake(20, 20, 20, 20);
      }

      list_t *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        node_t *child = children->items[i];
        NSView *view = node_render(child);

        if (view) {
          [stack addView:view
               inGravity:data->orientation ==
                                 node_type_box_orientation_type_vertical
                             ? NSStackViewGravityTop
                             : NSStackViewGravityLeading];
        }
      }

      return stack;
    }
    case node_type_button: {
      NSButton *button = [NSButton buttonWithTitle:@"" target:nil action:nil];
      node_type_button_data_t *data = node->data;

      if (data->clicked) {
        node_connect_event(button, node);
      }

      if (@available(macOS 26.0, *)) {
        button.bezelStyle = NSBezelStyleGlass;

        switch (data->variant) {
          case node_type_button_variant_type_primary: {
            button.tintProminence = NSTintProminencePrimary;
            break;
          }
          case node_type_button_variant_type_secondary: {
            button.tintProminence = NSTintProminenceNone;
            break;
          }
          case node_type_button_variant_type_destructive: {
            button.hasDestructiveAction = YES;
            break;
          }
          default: {
            break;
          }
        }
      }

      list_t *children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        node_t *child = children->items[i];

        if (child->type == node_type_text) {
          node_type_text_data_t *data = child->data;
          data->rendered = button;
          button.title = [NSString stringWithUTF8String:data->text];
        }
      }

      return button;
    }
    case node_type_text: {
      node_type_text_data_t *data = node->data;
      NSString *string = [NSString stringWithUTF8String:data->text];
      NSTextField *field = [NSTextField labelWithString:string];
      data->rendered = field;

      return field;
    }
  }

  return nil;
}

void node_type_alert_dialog_show(node_t *node) {
  node_type_alert_dialog_data_t *data = node->data;
  NSAlert *alert = data->rendered;
  [alert runModal];
}

void node_type_text_update(node_t *node, char *text) {
  node_type_text_data_t *data = node->data;
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

int node_type_window_run(node_t *node, int argc, char **argv) {
  (void)argc;
  (void)argv;

  NSString *bundleName =
      [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];

  NSString *name =
      bundleName ? bundleName : NSProcessInfo.processInfo.processName;

  NSApplication *app = NSApplication.sharedApplication;
  app.activationPolicy = NSApplicationActivationPolicyRegular;

  node_type_window_data_t *data = node->data;
  NSWindow *window = [[NSWindow alloc]
      initWithContentRect:NSMakeRect(0, 0, data->width, data->height)
                styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                           NSWindowStyleMaskMiniaturizable |
                           NSWindowStyleMaskResizable)
                  backing:NSBackingStoreBuffered
                    defer:NO];

  window.title = name;
  [window center];
  NSView *root = window.contentView;
  list_t *children = node->children;

  for (size_t i = 0; i < children->size; i++) {
    node_t *child = children->items[i];
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
  [app activateIgnoringOtherApps:YES];
  [app run];

  return 0;
}
