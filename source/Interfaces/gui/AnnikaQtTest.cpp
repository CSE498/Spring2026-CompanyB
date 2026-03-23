#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>

#include "Menu.hpp"

using Menu = cse498::Menu;

static void applyStyle(QWidget* window);
static void clearLayout(QLayout* layout);
static void rebuildMenuUI(QWidget* parent, QVBoxLayout* layout, Menu& menu);

static QVBoxLayout* buildMenuUI(QWidget* parent, Menu& menu)
{
    auto* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(30, 25, 30, 25);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    rebuildMenuUI(parent, layout, menu);
    return layout;
}

static void applyStyle(QWidget* window)
{
    window->setStyleSheet(R"(
        QWidget {
            background-color: #121212;
            color: #EAEAEA;
            font-family: "Helvetica Neue", Helvetica, Arial;
            font-size: 18px;
        }

        QLabel#MenuTitle {
            font-size: 22px;
            font-weight: 600;
            margin-bottom: 10px;
        }

        QPushButton {
            background-color: #1E1E1E;
            border: 1px solid #2A2A2A;
            border-radius: 12px;
            padding: 12px 14px;
        }

        QPushButton:hover {
            background-color: #252525;
            border: 1px solid #3A3A3A;
        }

        QPushButton:pressed {
            background-color: #2D2D2D;
        }

        QPushButton:disabled {
            color: #777777;
            background-color: #171717;
            border: 1px solid #222222;
        }
    )");
}

static void clearLayout(QLayout* layout)
{
    while (auto* child = layout->takeAt(0)) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}


static void rebuildMenuUI(QWidget* parent, QVBoxLayout* layout, Menu& menu)
{
    clearLayout(layout);

    auto* title = new QLabel(QString::fromStdString(menu.title()), parent);
    title->setObjectName("MenuTitle");
    title->setAlignment(Qt::AlignHCenter);
    layout->addWidget(title);

    const auto renderItems = menu.buildRenderModel();
    for (const auto& r : renderItems) {
        if (!r.visible) {
            continue;
        }

        auto* btn = new QPushButton(QString::fromStdString(r.label), parent);
        btn->setEnabled(r.enabled);
        btn->setFixedHeight(44);
        btn->setMinimumWidth(320);

        if (r.selected) {
            btn->setStyleSheet(
                "background-color: #2A2A2A; border: 1px solid #5A5A5A;");
        }

        QObject::connect(btn, &QPushButton::clicked, parent, [&menu, id = r.id]() {
            auto result = menu.activate(id);
            (void)result;
        });

        layout->addWidget(btn);
    }

    layout->addStretch(1);
}

// main generate window
int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Menu Test");
    window.resize(480, 360);
    applyStyle(&window);

    Menu menu("Test Menu");

    bool debugMode = false;
    QVBoxLayout* layout = nullptr;

    menu.addItem("info", "Info", [&window]() {
        QMessageBox::information(&window, "Info", "Menu action works!");
    });

    menu.addItem("quit", "Quit", [&app]() {
        app.quit();
    });

    auto debugId = menu.addItem("debug", "Debug Tools", [&window]() {
        QMessageBox::information(&window, "Debug", "Debug clicked!");
    });

    menu.setVisiblePredicate(debugId, [&debugMode]() {
        return debugMode;
    });

    menu.addItem("toggleDebug", "Toggle Debug", [&]() {
        debugMode = !debugMode;
        rebuildMenuUI(&window, layout, menu);
    });

    layout = buildMenuUI(&window, menu);
    rebuildMenuUI(&window, layout, menu);

    window.show();
    return app.exec();
}