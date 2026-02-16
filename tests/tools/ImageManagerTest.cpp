/**
 * @file ImageManagerTest.cpp
 * @author Andreea Danila
 */

#include "../../source/Interfaces/gui/ImageManager.hpp"

#include "../../source/Interfaces/gui/ImageManager.cpp"

#include <cassert>

int main() {

    // Test constructor
    cse498::ImageManager manager1;
    assert(!manager1.HasImage("anyImage"));
    assert(!manager1.HasImage(""));

    // Test first image
    manager1.Load("image1", "../../images/test1.png");
    assert(manager1.HasImage("image1"));
    assert(manager1.GetImage("image1") == "../../images/test1.png");

    // Test second image
    manager1.Load("image2", "../../images/test2.png");
    assert(manager1.HasImage("image1"));
    assert(manager1.HasImage("image2"));
    assert(manager1.GetImage("image2") == "../../images/test2.png");

    // Test AddToImages method
    manager1.AddToImages("image3", "/path/to/image3.png");
    assert(manager1.HasImage("image3"));
    assert(manager1.GetImage("image3") == "/path/to/image3.png");
    assert(manager1.HasImage("image1")); // Previous images still there
    assert(manager1.HasImage("image2"));

    // Test overwriting an existing image
    manager1.Load("image1", "/path/to/new_image1.png");
    assert(manager1.HasImage("image1"));
    assert(manager1.GetImage("image1") == "/path/to/new_image1.png");
    assert(manager1.GetImage("image1") != "/path/to/image1.png");

    // Test Remove
    manager1.Remove("image2");
    assert(!manager1.HasImage("image2"));
    assert(manager1.HasImage("image1"));
    assert(manager1.HasImage("image3"));

    // Test Clear
    manager1.Clear();
    assert(!manager1.HasImage("image1"));
    assert(!manager1.HasImage("image3"));

    // Test that we can add after clear
    manager1.AddToImages("newImage", "/path/new.png");
    assert(manager1.HasImage("newImage"));

    // Test empty ImageID
    cse498::ImageManager manager2;
    manager2.Load("", "/path/to/image.png");
    assert(!manager2.HasImage(""));

    // Test empty path
    manager2.Load("testImage", "");
    assert(!manager2.HasImage("testImage"));

    // Test AddToImages with empty ImageID
    manager2.AddToImages("", "/path/to/image.png");
    assert(!manager2.HasImage(""));

    // Test AddToImages with empty path
    manager2.AddToImages("testImage", "");
    assert(!manager2.HasImage("testImage"));

    return 0;
}