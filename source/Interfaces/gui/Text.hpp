#pragma once 

#include <QtCore/QString>
#include "QtGui/QFont"
#include "QtGui/QColor"
#include "QtWidgets/QGraphicsScene"

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
    Text(const QString& content, const QString& font, const QString& color, float size, bool bold, bool underline, bool italics, bool uppercase);

    // ------------------------------------getters------------------------------------------ //
    [[nodiscard]] QString GetContent() const;
    [[nodiscard]] QFont GetFont() const;
    [[nodiscard]] QColor GetColor() const;
    [[nodiscard]] float GetSize() const noexcept;
    [[nodiscard]] bool GetBold() const noexcept;
    [[nodiscard]] bool GetUnderline() const noexcept;
    [[nodiscard]] bool GetItalics() const noexcept;
    [[nodiscard]] bool GetUppercase() const noexcept;

    // ------------------------------------setters------------------------------------------ //
    void SetContent(const QString& content);
    void SetFont(const QString& font);
    void SetColor(const QString& color);
    void SetSize(float size);
    void SetBold(bool bold);
    void SetUnderline(bool underline);
    void SetItalics(bool italics);
    void SetUppercase(bool uppercase);

    // ------------------------------------display------------------------------------------ //
    void displayText(QGraphicsScene& scene);

};

}