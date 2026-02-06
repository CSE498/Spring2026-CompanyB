#pragma once 

#include <iostream>
#include <string>
using std::string;

namespace cse498 {

class Text {

private:

    // member variables 
    string mContent;

    string mFont;
    string mColor;
    int mSize;
    bool mBold;
    bool mUnderline;
    bool mItalics;
    bool mUppercase;

public:

    // constructor
    Text(string content, string font, string color, int size, bool bold, bool underline, bool italics, bool uppercase);

    // getters
    string GetContent();

    string GetFont();
    string GetColor();
    int GetSize();
    bool GetBold();
    bool GetUnderline();
    bool GetItalics();
    bool GetUppercase();

    // setters 
    void SetContent(string content);

    void SetFont(string font);
    void SetColor(string color);
    void SetSize(int size);
    void SetBold(bool bold);
    void SetUnderline(bool underline);
    void SetItalics(bool italics);
    void SetUppercase(bool uppercase);

};

}