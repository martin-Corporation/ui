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
  node_t *node;

  explicit NodeEventListener(node_t *node, QObject *parent = nullptr)
      : QObject(parent), node(node) {}

public slots:

  void handle() {
    auto data = (node_type_button_data_t *)node->data;

    if (data->clicked) {
      data->clicked();
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

static void node_render(node_t *node, QQuickItem *parent,
                        QQuickWindow *window) {
  switch (node->type) {
    case node_type_alert_dialog: {
      auto data = (node_type_alert_dialog_data_t *)node->data;
      auto dialog = create_object(engine, "QtQuick.Dialogs", "MessageDialog");
      dialog->setProperty("parentWindow", QVariant::fromValue(window));
      dialog->setProperty("text", QString::fromUtf8(data->title));
      data->rendered = dialog;

      if (data->description) {
        dialog->setProperty("informativeText",
                            QString::fromUtf8(data->description));
      }

      break;
    }
    case node_type_box: {
      auto wrapper =
          qobject_cast<QQuickItem *>(create_object(engine, "QtQuick", "Item"));

      auto data = (node_type_box_data_t *)node->data;
      int padding = data->padded ? 8 : 0;
      auto box = qobject_cast<QQuickItem *>(create_object(
          engine, "QtQuick.Layouts",
          (data->orientation == node_type_box_orientation_type_horizontal)
              ? "RowLayout"
              : "ColumnLayout"));

      box->setProperty("spacing", data->spaced ? 8 : 0);
      wrapper->setParentItem(parent);
      box->setParentItem(wrapper);

      auto anchors = box->property("anchors").value<QObject *>();
      anchors->setProperty("fill", QVariant::fromValue(wrapper));
      anchors->setProperty("margins", padding);

      for (size_t i = 0; i < node->children->size; i++) {
        auto child = (node_t *)node->children->items[i];
        node_render(child, box, window);
      }

      bind_implicit_size(wrapper, box, "implicitWidth", padding);
      bind_implicit_size(wrapper, box, "implicitHeight", padding);

      break;
    }
    case node_type_button: {
      auto button = qobject_cast<QQuickItem *>(
          create_object(engine, "QtQuick.Controls", "Button"));

      auto data = (node_type_button_data_t *)node->data;
      button->setParentItem(parent);

      if (data->clicked) {
        auto listener = new NodeEventListener(node, button);
        QObject::connect(button, SIGNAL(clicked()), listener, SLOT(handle()));
      }

      if (data->variant == node_type_button_variant_type_primary) {
        button->setProperty("highlighted", true);
      }

      for (size_t i = 0; i < node->children->size; i++) {
        auto child = (node_t *)node->children->items[i];

        if (child->type == node_type_text) {
          auto data = (node_type_text_data_t *)child->data;
          data->rendered = button;
          button->setProperty("text", QString::fromUtf8(data->text));
        } else {
          node_render(child, button, window);
        }
      }

      break;
    }
    case node_type_text: {
      auto data = (node_type_text_data_t *)node->data;
      auto text =
          qobject_cast<QQuickItem *>(create_object(engine, "QtQuick", "Text"));

      data->rendered = text;
      text->setProperty("color", QGuiApplication::palette().windowText());
      text->setProperty("text", QString::fromUtf8(data->text));
      text->setParentItem(parent);

      break;
    }
    default: {
      break;
    }
  }
}

extern "C" void node_type_alert_dialog_show(node_t *node) {
  auto data = (node_type_alert_dialog_data_t *)node->data;
  auto dialog = (QObject *)data->rendered;
  QMetaObject::invokeMethod(dialog, "open");
}

extern "C" void node_type_text_update(node_t *node, char *text) {
  auto data = (node_type_text_data_t *)node->data;
  data->text = text;

  if (data->rendered) {
    auto label = (QQuickItem *)data->rendered;
    label->setProperty("text", QString::fromUtf8(data->text));
  }
}

extern "C" int node_type_window_run(node_t *node, int argc, char **argv) {
  auto data = (node_type_window_data_t *)node->data;
  QGuiApplication app(argc, argv);
  QQuickWindow window;
  QQmlEngine _engine;
  engine = &_engine;

  app.setDesktopFileName(QString::fromUtf8(data->id));
  window.setColor(app.palette().window().color());
  window.setHeight(data->height);
  window.setWidth(data->width);

  for (size_t i = 0; i < node->children->size; i++) {
    auto child = (node_t *)node->children->items[i];
    node_render(child, window.contentItem(), &window);
  }

  window.show();
  return app.exec();
}

#include "qt.moc"
