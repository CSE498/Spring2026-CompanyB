#include "Text.hpp"

namespace cse498 {

// constructor 
Text::Text(string content, string font, string color, int size, bool bold, bool underline, bool italics, bool glowing, bool uppercase){
    mContent = content;

    mFont = font;
    mColor = color;
    mSize = size;
    mBold = bold;
    mUnderline = underline;
    mItalics = italics;
    mGlowing = glowing;
    mUppercase = uppercase;
}

// getters
string Text::GetContent(){ return mContent; } 

string Text::GetFont(){ return mFont; } 
string Text::GetColor(){ return mColor; } 
int Text::GetSize(){ return mSize; } 
bool Text::GetBold(){ return mBold; } 
bool Text::GetUnderline(){ return mUnderline; } 
bool Text::GetItalics(){ return mItalics; } 
bool Text::GetGlowing(){ return mGlowing; } 
bool Text::GetUppercase(){ return mUppercase; } 

// setters
void Text::SetContent(string content){ mContent = content; } 

void Text::SetFont(string font){ mFont = font; } 
void Text::SetColor(string color){ mColor = color; } 
void Text::SetSize(int size){ mSize = size; } 
void Text::SetBold(bool bold){ mBold = bold; } 
void Text::SetUnderline(bool underline){ mUnderline = underline; } 
void Text::SetItalics(bool italics){ mItalics = italics; } 
void Text::SetGlowing(bool glowing){ mGlowing = glowing; } 
void Text::SetUppercase(bool uppercase){ mUppercase = uppercase; } 

}