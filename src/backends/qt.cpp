#include <QQmlContext>
#include <QQmlExpression>
#include <QtQuick/QtQuick>
#include <node.h>

static QQmlEngine *engine;

static QObject *create_object(QQmlEngine *engine, const char *module,
                              const char *type) {
  QQmlComponent component(engine);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  component.loadFromModule(module, type);
#else
  QByteArray source = QString("import %1\n%2 {}").arg(module, type).toUtf8();
  component.setData(source, QUrl());
#endif

  if (component.isError()) {
    qWarning() << component.errors();
    return nullptr;
  }

  auto object = component.create();

  if (!object) {
    qWarning() << component.errors();
  }

  return object;
}

class NodeEventListener : public QObject {
  Q_OBJECT

public:
  Node *node;

  explicit NodeEventListener(Node *node, QObject *parent = nullptr)
      : QObject(parent), node(node) {}

public slots:

  void handle() {
    switch (node->type) {
      case NodeTypeButton: {
        auto data = (NodeTypeButtonData *)node->data;

        if (data->clicked != nullptr) {
          data->clicked();
        }

        break;
      }
      case NodeTypeAlertDialog: {
        auto data = (NodeTypeAlertDialogData *)node->data;

        if (data->handler != nullptr) {
          data->handler();
        }

        break;
      }
      default: {
        break;
      }
    }
  }
};

static void bind_implicit_size(QQuickItem *wrapper, QQuickItem *content,
                               const char *property, int padding) {
  auto expression = new QQmlExpression(
      engine->rootContext(), content,
      QString("%1 + %2").arg(property).arg(padding * 2), wrapper);

  expression->setNotifyOnValueChanged(true);
  auto update = [expression, wrapper, property]() {
    wrapper->setProperty(property, expression->evaluate());
  };

  QObject::connect(expression, &QQmlExpression::valueChanged, wrapper, update);
  update();
}

static void node_render(Node *node, QQuickItem *parent, QQuickWindow *window) {
  switch (node->type) {
    case NodeTypeAlertDialog: {
      auto dialog = create_object(engine, "QtQuick.Dialogs", "MessageDialog");
      auto data = (NodeTypeAlertDialogData *)node->data;
      dialog->setProperty("parentWindow", QVariant::fromValue(window));
      dialog->setProperty("text", QString::fromUtf8(data->title));
      data->rendered = dialog;

      if (data->handler != NULL) {
        auto listener = new NodeEventListener(node, dialog);
        QObject::connect(dialog, SIGNAL(accepted()), listener, SLOT(handle()));
      }

      if (data->description) {
        dialog->setProperty("informativeText",
                            QString::fromUtf8(data->description));
      }

      break;
    }
    case NodeTypeBox: {
      auto wrapper =
          qobject_cast<QQuickItem *>(create_object(engine, "QtQuick", "Item"));

      auto data = (NodeTypeBoxData *)node->data;
      int padding = data->padded ? 8 : 0;
      auto box = qobject_cast<QQuickItem *>(create_object(
          engine, "QtQuick.Layouts",
          (data->orientation == NodeTypeBoxOrientationTypeHorizontal)
              ? "RowLayout"
              : "ColumnLayout"));

      box->setProperty("spacing", data->spaced ? 8 : 0);
      wrapper->setParentItem(parent);
      box->setParentItem(wrapper);

      auto anchors = box->property("anchors").value<QObject *>();
      anchors->setProperty("fill", QVariant::fromValue(wrapper));
      anchors->setProperty("margins", padding);

      auto children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        auto child = (Node *)children->items[i];
        node_render(child, box, window);
      }

      bind_implicit_size(wrapper, box, "implicitWidth", padding);
      bind_implicit_size(wrapper, box, "implicitHeight", padding);

      break;
    }
    case NodeTypeButton: {
      auto button = qobject_cast<QQuickItem *>(
          create_object(engine, "QtQuick.Controls", "Button"));

      auto data = (NodeTypeButtonData *)node->data;
      button->setParentItem(parent);

      if (data->clicked) {
        auto listener = new NodeEventListener(node, button);
        QObject::connect(button, SIGNAL(clicked()), listener, SLOT(handle()));
      }

      if (data->variant == NodeTypeButtonVariantTypePrimary) {
        button->setProperty("highlighted", true);
      }

      auto children = node->children;

      for (size_t i = 0; i < children->size; i++) {
        auto child = (Node *)children->items[i];

        if (child->type == NodeTypeText) {
          auto data = (NodeTypeTextData *)child->data;
          data->rendered = button;
          button->setProperty("text", QString::fromUtf8(data->text));
        } else {
          node_render(child, button, window);
        }
      }

      break;
    }
    case NodeTypeText: {
      auto data = (NodeTypeTextData *)node->data;
      auto text = qobject_cast<QQuickItem *>(create_object(
          engine, "QtQuick", data->selectable ? "TextEdit" : "Text"));

      if (data->selectable) {
        text->setProperty("selectByKeyboard", true);
        text->setProperty("selectByMouse", true);
        text->setProperty("readOnly", true);
      }

      text->setProperty("color", QGuiApplication::palette().windowText());
      text->setProperty("text", QString::fromUtf8(data->text));
      text->setParentItem(parent);
      data->rendered = text;

      break;
    }
    default: {
      break;
    }
  }
}

extern "C" void node_type_alert_dialog_show(Node *node) {
  auto data = (NodeTypeAlertDialogData *)node->data;
  auto dialog = (QObject *)data->rendered;
  QMetaObject::invokeMethod(dialog, "open");
}

extern "C" void node_type_text_update(Node *node, char *text) {
  auto data = (NodeTypeTextData *)node->data;
  data->text = text;

  if (data->rendered) {
    auto label = (QQuickItem *)data->rendered;
    label->setProperty("text", QString::fromUtf8(data->text));
  }
}

extern "C" int node_type_window_run(Node *node, int argc, char **argv) {
  auto data = (NodeTypeWindowData *)node->data;
  QGuiApplication app(argc, argv);
  QQuickWindow window;
  QQmlEngine _engine;
  engine = &_engine;

  app.setDesktopFileName(QString::fromUtf8(data->id));
  window.setColor(app.palette().window().color());
  window.setHeight(data->height);
  window.setWidth(data->width);

  auto children = node->children;

  for (size_t i = 0; i < children->size; i++) {
    auto child = (Node *)children->items[i];
    node_render(child, window.contentItem(), &window);
  }

  window.show();
  return app.exec();
}

#include "qt.moc"
