#pragma once 

#include "../../../third-party/qt/include/QtCore/QString"
#include "../../../third-party/qt/include/QtGui/QFont"
#include "../../../third-party/qt/include/QtGui/QColor"
#include "../../../third-party/qt/include/QtWidgets/QGraphicsScene"

namespace cse498 {

class Text {

private:

    // member variables 
    QString mContent;
    QFont mFont;
    QColor mColor;
    float mSize;
    bool mBold;
    bool mUnderline;
    bool mItalics;
    bool mUppercase;

public:

    // ----------------------------------constructor---------------------------------------- //
    Text(QString content, QString font, QString color, float size, bool bold, bool underline, bool italics, bool uppercase);

    // ------------------------------------getters------------------------------------------ //
    QString GetContent();
    QFont GetFont();
    QColor GetColor();
    float GetSize();
    bool GetBold();
    bool GetUnderline();
    bool GetItalics();
    bool GetUppercase();

    // ------------------------------------setters------------------------------------------ //
    void SetContent(QString content);
    void SetFont(QString font);
    void SetColor(QString color);
    void SetSize(float size);
    void SetBold(bool bold);
    void SetUnderline(bool underline);
    void SetItalics(bool italics);
    void SetUppercase(bool uppercase);

    // ------------------------------------display------------------------------------------ //
    void displayText(QGraphicsScene& scene);

};

}