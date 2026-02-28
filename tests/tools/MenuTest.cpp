#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../../source/Interfaces/gui/Menu.hpp"
#include <string>

using cse498::Menu;

TEST_CASE("Menu basics default and title ctor", "[gui][menu]")
{
    Menu m;
    CHECK(m.title().empty());
    CHECK(m.isOpen());
    CHECK(m.ignoreDisabledActivation());

    Menu mt("Main Menu");
    CHECK(mt.title() == std::string("Main Menu"));
}

TEST_CASE("Menu setters getters open ignoreDisabledActivation", "[gui][menu]")
{
    Menu m;

    m.setTitle("Main Menu");
    CHECK(m.title() == std::string("Main Menu"));

    m.setIgnoreDisabledActivation(false);
    CHECK_FALSE(m.ignoreDisabledActivation());

    m.setIgnoreDisabledActivation(true);
    CHECK(m.ignoreDisabledActivation());

    m.setOpen(false);
    CHECK_FALSE(m.isOpen());

    m.setOpen(true);
    CHECK(m.isOpen());
}

TEST_CASE("Menu add get setters remove clear invalid ids", "[gui][menu]")
{
    Menu m;

    auto id = m.addItem("x", "X", []{}, true, true);
    REQUIRE(id != 0);
    REQUIRE(m.items().size() == 1);

    auto* it = m.getItem(id);
    REQUIRE(it != nullptr);
    CHECK(it->key == "x");
    CHECK(it->label == "X");
    CHECK(it->enabled);
    CHECK(it->visible);

    m.setItemLabel(id, "New");
    m.setItemEnabled(id, false);
    m.setItemVisible(id, false);

    const auto* cit = m.getItem(id);
    REQUIRE(cit != nullptr);
    CHECK(cit->label == "New");
    CHECK_FALSE(cit->enabled);
    CHECK_FALSE(cit->visible);

    CHECK(m.getItem(999999u) == nullptr);
    CHECK_FALSE(m.select(999999u));
    CHECK_FALSE(m.activate(999999u));
    CHECK_FALSE(m.removeItem(999999u));
    CHECK_FALSE(m.hover(999999u));
    m.setItemEnabled(999999u, true);
    m.setItemVisible(999999u, true);
    m.setItemLabel(999999u, "nope");
    m.setEnabledPredicate(999999u, [] { return true; });
    m.setVisiblePredicate(999999u, [] { return true; });
    m.setActivateAction(999999u, [] {});
    m.setHoverAction(999999u, [] {});
    m.setSelectedAction(999999u, [] {});

    CHECK(m.removeItem(id));
    CHECK(m.getItem(id) == nullptr);

    m.clear();
    CHECK(m.items().empty());
}

TEST_CASE("Menu key helpers find get remove", "[gui][menu]")
{
    Menu m;

    auto id = m.addItem("alpha", "Alpha", []{}, true, true);

    auto opt = m.findItemIdByKey("alpha");
    REQUIRE(opt.has_value());
    CHECK(*opt == id);

    CHECK(m.getItemByKey("alpha") != nullptr);
    CHECK(m.getItemByKey("missing") == nullptr);

    CHECK_FALSE(m.removeItem("missing"));
    CHECK(m.removeItem("alpha"));
    CHECK(m.items().empty());
}

TEST_CASE("Menu render model checks all fields", "[gui][menu]")
{
    Menu m;

    auto id1 = m.addItem("one", "One", []{}, true, true);
    auto id2 = m.addItem("two", "Two", []{}, true, true);

    auto rm = m.buildRenderModel();
    REQUIRE(rm.size() == 2);

    CHECK(rm[0].id == id1);
    CHECK(rm[0].label == "One");
    CHECK(rm[0].enabled == true);
    CHECK(rm[0].visible == true);
    CHECK(rm[0].selected == false);

    CHECK(rm[1].id == id2);
    CHECK(rm[1].label == "Two");
    CHECK(rm[1].enabled == true);
    CHECK(rm[1].visible == true);
    CHECK(rm[1].selected == false);
}

TEST_CASE("Menu predicates hover invisible hover select transition activate guards", "[gui][menu]")
{
    Menu m;

    int act = 0;
    int hov = 0;
    int sel = 0;

    auto id1 = m.addItem("one", "One", [&]{ act++; }, true, true);
    auto id2 = m.addItem("two", "Two", []{}, true, true);

    bool allowEnabled = true;
    bool allowVisible = true;

    m.setEnabledPredicate(id1, [&]{ return allowEnabled; });
    m.setVisiblePredicate(id1, [&]{ return allowVisible; });

    m.setHoverAction(id1, [&]{ hov++; });
    m.setSelectedAction(id2, [&]{ sel++; });

    {
        auto rm = m.buildRenderModel();
        REQUIRE(rm.size() == 2);
        CHECK(rm[0].enabled == true);
        CHECK(rm[0].visible == true);
    }

    allowEnabled = false;
    CHECK_FALSE(m.buildRenderModel()[0].enabled);

    allowEnabled = true;
    allowVisible = false;
    CHECK_FALSE(m.buildRenderModel()[0].visible);

    allowVisible = true;
    CHECK(m.hover(id1));
    CHECK(hov == 1);

    allowVisible = false;
    CHECK_FALSE(m.hover(id1));
    CHECK(hov == 1);

    CHECK(m.select(id2));
    CHECK(sel == 1);
    CHECK(m.select(id2));
    CHECK(sel == 1);

    m.setItemVisible(id1, false);
    CHECK_FALSE(m.activate(id1));
    CHECK(act == 0);

    m.setItemVisible(id1, true);
    m.setItemEnabled(id1, false);
    m.setIgnoreDisabledActivation(true);
    CHECK_FALSE(m.activate(id1));
    CHECK(act == 0);

    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id1));
    CHECK(act == 1);
}

TEST_CASE("Menu duplicate key updates item and action", "[gui][menu]")
{
    Menu m;

    int callsA = 0;
    int callsB = 0;

    auto id1 = m.addItem("k", "Label1", [&]{ callsA++; }, true, true);
    auto id2 = m.addItem("k", "Label2", [&]{ callsB++; }, false, false);

    CHECK(id1 == id2);
    REQUIRE(m.items().size() == 1);

    const auto* it = m.getItem(id1);
    REQUIRE(it != nullptr);
    CHECK(it->label == "Label2");
    CHECK_FALSE(it->enabled);
    CHECK_FALSE(it->visible);

    m.setItemVisible(id1, true);
    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id1));
    CHECK(callsA == 0);
    CHECK(callsB == 1);
}

TEST_CASE("Menu setActivateAction activates updated callback", "[gui][menu]")
{
    Menu m;

    int a = 0;
    int b = 0;

    auto id1 = m.addItem("a", "A", [&]{ a++; }, true, true);
    auto id2 = m.addItem("b", "B", [&]{ b++; }, true, true);

    m.setActivateAction(id2, [&]{ b += 10; });

    CHECK(m.select(id2));
    CHECK(m.activate(id2));
    CHECK(a == 0);
    CHECK(b == 10);

    CHECK(m.select(id1));
    CHECK(m.activate(id1));
    CHECK(a == 1);
    CHECK(b == 10);
}

TEST_CASE("Menu selection navigation functions called", "[gui][menu]")
{
    Menu m;

    int a = 0;
    int b = 0;
    int c = 0;

    auto id1 = m.addItem("a", "A", [&]{ a++; }, true, true);
    auto id2 = m.addItem("b", "B", [&]{ b++; }, true, true);
    auto id3 = m.addItem("c", "C", [&]{ c++; }, true, true);

    (void)id2;
    (void)id3;

    CHECK(m.selectFirst());
    CHECK(m.activateSelected());
    CHECK(a == 1);

    CHECK(m.selectLast());
    CHECK(m.activateSelected());
    CHECK(c == 1);

    CHECK(m.select(id1));
    CHECK(m.selectNext());
    CHECK(m.activateSelected());
    CHECK(b == 1);

    CHECK(m.selectPrevious());
    CHECK(m.activateSelected());
    CHECK(a == 2);
}

TEST_CASE("Menu empty menu selection navigation safe", "[gui][menu]")
{
    Menu m;

    CHECK_FALSE(m.selectFirst());
    CHECK_FALSE(m.selectLast());
    CHECK_FALSE(m.selectNext());
    CHECK_FALSE(m.selectPrevious());
    CHECK_FALSE(m.activateSelected());
}

TEST_CASE("Menu handleNav covers all events", "[gui][menu]")
{
    Menu m;

    auto id1 = m.addItem("a", "A", []{}, true, true);
    auto id2 = m.addItem("b", "B", []{}, true, true);

    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::Up) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::Left) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::Right) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::Home) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::End) == Menu::InputResult::Heard);
    CHECK(m.handleNav(Menu::NavEvent::Activate) == Menu::InputResult::Heard);

    CHECK(m.isOpen());
    CHECK(m.handleNav(Menu::NavEvent::Back) == Menu::InputResult::Heard);
    CHECK_FALSE(m.isOpen());

    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Ignored);

    (void)id1;
    (void)id2;
}