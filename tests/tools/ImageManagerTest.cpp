/**
 * @file ImageManagerTest.cpp
 * @author Andreea Danila
 */
//To run 'g++ ImageManagerTest.cpp ..\..\source\Interfaces\gui\ImageManager.cpp -o ImageManagerTest'
//To run test write  './ImageManagerTest'

#include "catch2/catch.hpp"

#include "../../source/Interfaces/gui/ImageManager.hpp"

TEST_CASE("Test ImageManager Constructor", "[ImageManager]")
{
    cse498::ImageManager manager1;
    REQUIRE(!manager1.HasImage("anyImage"));
    REQUIRE(!manager1.HasImage(""));
}


TEST_CASE("Test ImageManager Remove", "[ImageManager]")
{
    cse498::ImageManager manager1;
    manager1.Load("image1", "../../images/test1.png");
    manager1.Load("image2", "../../images/test2.png");

    manager1.Remove("image2");
    REQUIRE(!manager1.HasImage("image2"));
    REQUIRE(manager1.HasImage("image1"));
}

TEST_CASE("Test ImageManager Clear", "[ImageManager]")
{
    cse498::ImageManager manager1;
    manager1.Load("image1", "../../images/test1.png");

    manager1.Clear();
    REQUIRE(!manager1.HasImage("image1"));

}
