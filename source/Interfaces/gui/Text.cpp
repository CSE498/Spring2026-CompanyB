#include "Text.hpp"

#include "../../../third-party/qt/include/QtCore/QString"
#include "../../../third-party/qt/include/QtGui/QFont"
#include "../../../third-party/qt/include/QtGui/QColor"

// for sample display
#include "../../../third-party/qt/include/QtWidgets/QApplication"
#include "../../../third-party/qt/include/QtWidgets/QPushButton"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsTextItem"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsView"

namespace cse498 {

// ----------------------------------constructor---------------------------------------- //
Text::Text(QString content, QString font, QString color, float size, bool bold, bool underline, bool italics, bool uppercase){
    mContent = content;
    mFont = QFont(font);
    mColor = QColor(color);
    mSize = size;
    mBold = bold;
    mUnderline = underline;
    mItalics = italics;
    mUppercase = uppercase;

    mFont.setPointSizeF(size);
    mFont.setBold(bold);
    mFont.setUnderline(underline);
    mFont.setItalic(italics);

    if (uppercase == true) 
        mFont.setCapitalization(QFont::AllUppercase);
    else 
        mFont.setCapitalization(QFont::MixedCase);
}

// ------------------------------------getters------------------------------------------ //
QString Text::GetContent(){ return mContent; } 

QFont Text::GetFont(){ return mFont; } 
QColor Text::GetColor(){ return mColor; } 
float Text::GetSize(){ return mSize; } 
bool Text::GetBold(){ return mBold; } 
bool Text::GetUnderline(){ return mUnderline; } 
bool Text::GetItalics(){ return mItalics; } 
bool Text::GetUppercase(){ return mUppercase; } 

// ------------------------------------setters------------------------------------------ //
void Text::SetContent(QString content){ mContent = content; } 

void Text::SetFont(QString font){ mFont = QFont(font); } 
void Text::SetColor(QString color){ mColor = QColor(color); } 

void Text::SetSize(float size){ 
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

    if (uppercase == true) 
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
*/

/* the texts do overlap when displayed, but we are still working on the GUI so it is currently
   (or maybe completely) out of my jurisdiction to modify displayText to take in positions */

/*
view.show();

return app.exec();
}
*/
// ------------------------------------------------------------------------------------- //