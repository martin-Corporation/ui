#include <QtQuick/QtQuick>
#include <node.h>

static QQmlEngine *engine;

static QQuickItem *create_item(QQmlEngine *engine, const char *module,
                               const char *type) {
  QQmlComponent component(engine);
  component.loadFromModule(module, type);
  QObject *object = component.create();

  return qobject_cast<QQuickItem *>(object);
}

void node_render(node_t *node, QQuickItem *parent) {
  switch (node->type) {
  case node_type_box: {
    auto box = create_item(engine, "QtQuick.Layouts",
                           (*(node_type_box_orientation_type_t *)node->data ==
                            node_type_box_orientation_type_horizontal)
                               ? "RowLayout"
                               : "ColumnLayout");

    box->setParentItem(parent);

    for (size_t i = 0; i < node->children->size; i++) {
      node_render((node_t *)node->children->items[i], box);
    }

    break;
  }
  case node_type_button: {
    auto data = *(node_type_button_variant_type_t *)node->data;
    auto button = create_item(engine, "QtQuick.Controls", "Button");
    button->setParentItem(parent);

    if (data == node_type_button_variant_type_primary) {
      button->setProperty("highlighted", true);
    }

    for (size_t i = 0; i < node->children->size; i++) {
      auto child = (node_t *)node->children->items[i];

      if (child->type == node_type_text) {
        button->setProperty("text", QString::fromUtf8((char *)child->data));
      } else {
        node_render(child, button);
      }
    }

    break;
  }
  case node_type_text: {
    auto text = create_item(engine, "QtQuick", "Text");
    text->setProperty("text", QString::fromUtf8((char *)node->data));
    text->setProperty("color", QGuiApplication::palette().windowText());
    text->setParentItem(parent);

    break;
  }
  default: {
    break;
  }
  }
}

extern "C" int node_run(node_t *node, int argc, char **argv) {
  QGuiApplication app(argc, argv);
  QQmlEngine _engine;
  QQuickWindow window;
  engine = &_engine;

  app.setDesktopFileName(QString::fromUtf8((char *)node->data));
  window.setColor(app.palette().window().color());
  window.setWidth(500);
  window.setHeight(250);

  for (size_t i = 0; i < node->children->size; i++) {
    node_render((node_t *)node->children->items[i], window.contentItem());
  }

  window.show();
  return app.exec();
}
