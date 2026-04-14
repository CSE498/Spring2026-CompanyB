#include "Text.hpp"

#include <cassert>

// #include "../../../third-party/qt/include/QtCore/QString"
#include <QString>
// #include "../../../third-party/qt/include/QtGui/QFont"
#include <QFont>
// #include "../../../third-party/qt/include/QtGui/QColor"
#include <QColor>

// for sample display
// #include "../../../third-party/qt/include/QtWidgets/QApplication"
#include <QApplication>
// #include "../../../third-party/qt/include/QtWidgets/QPushButton"
#include <QPushButton>
// #include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"
#include <QGraphicsScene>
// #include "../../../third-party/qt/include/QtWidgets/QGraphicsTextItem"
#include <QGraphicsTextItem>
// #include "../../../third-party/qt/include/QtWidgets/QGraphicsView"
#include <QGraphicsView>

namespace cse498 {

// ----------------------------------constructor---------------------------------------- //
Text::Text(const QString& content, const QString& font, const QString& color, float size, bool bold, bool underline, bool italics, bool uppercase) 
: mContent(content), mFont(font), mColor(color), mSize(size), mBold(bold), mUnderline(underline), mItalics(italics), mUppercase(uppercase) {

    assert(size > 0); // font size has to be a positive value
    mFont.setPointSizeF(size);

    mFont.setBold(bold);
    mFont.setUnderline(underline);
    mFont.setItalic(italics);

    if (uppercase) 
        mFont.setCapitalization(QFont::AllUppercase);
    else 
        mFont.setCapitalization(QFont::MixedCase);
}

// ------------------------------------getters------------------------------------------ //
QString Text::GetContent() const { return mContent; } 

QFont Text::GetFont() const { return mFont; } 
QColor Text::GetColor() const { return mColor; } 
float Text::GetSize() const noexcept { return mSize; } 
bool Text::GetBold() const noexcept { return mBold; } 
bool Text::GetUnderline() const noexcept { return mUnderline; } 
bool Text::GetItalics() const noexcept { return mItalics; } 
bool Text::GetUppercase() const noexcept { return mUppercase; } 

// ------------------------------------setters------------------------------------------ //
void Text::SetContent(const QString& content){ mContent = content; } 

void Text::SetFont(const QString& font){ mFont = QFont(font); } 
void Text::SetColor(const QString& color){ mColor = QColor(color); } 

void Text::SetSize(float size){ 
    assert(size > 0);
    mSize = size;
    mFont.setPointSizeF(size);
 }

void Text::SetBold(bool bold){
    mBold = bold;
    mFont.setBold(bold);
} 
void Text::SetUnderline(bool underline){ 
    mUnderline = underline; 
    mFont.setUnderline(underline);
} 
void Text::SetItalics(bool italics){ 
    mItalics = italics; 
    mFont.setItalic(italics);
} 

void Text::SetUppercase(bool uppercase){ 
    mUppercase = uppercase; 

    if (uppercase) 
        mFont.setCapitalization(QFont::AllUppercase);
    else 
        mFont.setCapitalization(QFont::MixedCase);
} 

// ------------------------------------display------------------------------------------ //
void Text::displayText(QGraphicsScene& scene){
    QGraphicsTextItem* textItem = scene.addText(mContent);
    textItem->setFont(mFont);
    textItem->setDefaultTextColor(mColor);
}
}

// -------------------------------------main-------------------------------------------- //
// this main function won't be in the final class implementation, it exists to test out the class

/*
int main(int argc, char **argv)
{
 QApplication app (argc, argv);

QGraphicsScene scene;
QGraphicsView view(&scene);

cse498::Text helloText("hello", "Arial", "blue", 30, true, false, true, true);
helloText.displayText(scene);

cse498::Text byeText("bye", "Garamond", "#751401", 50, false, true, false, false);
byeText.displayText(scene);

view.show();

return app.exec();
}
*/

/* the texts do overlap when displayed, but we are still working on the GUI so it is currently
   (or maybe completely) out of my jurisdiction to modify displayText to take in positions */

// ------------------------------------------------------------------------------------- //