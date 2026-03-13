///**
// * @file ImageManagerTest.cpp
// * @author Andreea Danila
// */
////To run 'g++ ImageManagerTest.cpp ..\..\source\Interfaces\gui\ImageManager.cpp -o ImageManagerTest'
////To run test write  './ImageManagerTest'
//
//
//#define CATCH_CONFIG_MAIN
//#include "../../third-party/Catch/single_include/catch2/catch.hpp"
//#include <QApplication>
//#include "../../source/Interfaces/gui/ImageManager.hpp"
//
//
//static int argc = 0;
//static QApplication app(argc, nullptr);
//
//TEST_CASE("Test ImageManager Constructor", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//    CHECK(!manager1.HasImage("anyImage"));
//    CHECK(!manager1.HasImage(""));
//}
//
//TEST_CASE("Test ImageManager Load", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//
//    // testing valid load
//    CHECK(manager1.Load("image1", "../../images/test1.png") == true);
//    CHECK(manager1.HasImage("image1"));
//
//    // testing empty id
//    CHECK(manager1.Load("", "../../images/test1.png") == false);
//
//    // testing empty path
//    CHECK(manager1.Load("image2", "") == false);
//    CHECK(!manager1.HasImage("image2"));
//
//    // testing invalid path
//    CHECK(manager1.Load("image3", "fake/path.png") == false);
//    CHECK(!manager1.HasImage("image3"));
//
//    // testing overwrite same id
//    CHECK(manager1.Load("image1", "../../images/test2.png") == true);
//    CHECK(manager1.HasImage("image1"));
//}
//
//TEST_CASE("Test ImageManager HasImage", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//
//    // before loading
//    CHECK(!manager1.HasImage("image1"));
//
//    // after loading
//    manager1.Load("image1", "../../images/test1.png");
//    CHECK(manager1.HasImage("image1"));
//
//    // image that was never loaded
//    CHECK(!manager1.HasImage("image2"));
//}
//
//TEST_CASE("Test ImageManager GetImage", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//    manager1.Load("image1", "../../images/test1.png");
//
//    // existing image does not throw
//    CHECK_NOTHROW(manager1.GetImage("image1"));
//
//    // missing image throws out_of_range
//    CHECK_THROWS_AS(manager1.GetImage("missing"), std::out_of_range);
//}
//
//TEST_CASE("Test ImageManager Remove", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//    manager1.Load("image1", "../../images/test1.png");
//    manager1.Load("image2", "../../images/test2.png");
//
//    // remove existing image
//    manager1.Remove("image2");
//    CHECK(!manager1.HasImage("image2"));
//    CHECK(manager1.HasImage("image1"));
//
//    // remove nonexistent image does not crash
//    CHECK_NOTHROW(manager1.Remove("doesNotExist"));
//
//    // remove then re-add
//    manager1.Remove("image1");
//    CHECK(!manager1.HasImage("image1"));
//    manager1.Load("image1", "../../images/test1.png");
//    CHECK(manager1.HasImage("image1"));
//}
//
//TEST_CASE("Test ImageManager Clear", "[ImageManager]")
//{
//    cse498::ImageManager manager1;
//    manager1.Load("image1", "../../images/test1.png");
//    manager1.Load("image2", "../../images/test2.png");
//
//    manager1.Clear();
//    CHECK(!manager1.HasImage("image1"));
//    CHECK(!manager1.HasImage("image2"));
//
//    // clear already empty manager does not crash
//    CHECK_NOTHROW(manager1.Clear());
//}