#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../../source/Interfaces/gui/Menu.hpp"
#include <string>

using cse498::Menu;

TEST_CASE("Menu basics: title/open/ignoreDisabledActivation", "[gui][menu]")
{
    Menu m;
    CHECK(m.title().empty());
    CHECK(m.isOpen());
    CHECK(m.ignoreDisabledActivation());

    m.setTitle("Main Menu");
    CHECK(m.title() == std::string("Main Menu"));

    m.setOpen(false);
    CHECK_FALSE(m.isOpen());
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Ignored);

    m.setOpen(true);
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Heard);

    m.setIgnoreDisabledActivation(false);
    CHECK_FALSE(m.ignoreDisabledActivation());
}

TEST_CASE("Menu add/get + setters + remove/clear", "[gui][menu]")
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

    CHECK_FALSE(m.removeItem(999999u));
    CHECK(m.removeItem(id));
    CHECK(m.getItem(id) == nullptr);

    m.clear();
    CHECK(m.items().empty());
}

TEST_CASE("Menu predicates + hover/select/activate rules", "[gui][menu]")
{
    Menu m;
    int act = 0, hov = 0, sel = 0;

    auto id1 = m.addItem("one", "One", [&]{ act++; }, true, true);
    auto id2 = m.addItem("two", "Two", []{}, true, true);

    bool allowEnabled = true, allowVisible = true;
    m.setEnabledPredicate(id1, [&]{ return allowEnabled; });
    m.setVisiblePredicate(id1, [&]{ return allowVisible; });

    m.setHoverAction(id1, [&]{ hov++; });
    m.setSelectedAction(id2, [&]{ sel++; });

    // render model uses predicates
    {
        auto rm = m.buildRenderModel();
        REQUIRE(rm.size() == 2);
        CHECK(rm[0].enabled);
        CHECK(rm[0].visible);
    }
    allowEnabled = false;
    CHECK_FALSE(m.buildRenderModel()[0].enabled);

    allowVisible = false;
    CHECK_FALSE(m.buildRenderModel()[0].visible);

    // hover (only if visible)
    allowVisible = true;
    m.hover(id1);
    CHECK(hov == 1);

    // select only triggers once per transition
    CHECK(m.select(id2));
    CHECK(sel == 1);
    CHECK(m.select(id2));
    CHECK(sel == 1);

    // activate rules invisible never activates
    m.setItemVisible(id1, false);
    CHECK_FALSE(m.activate(id1));
    CHECK(act == 0);

    // disabled blocks when ignoreDisabledActivation=true
    m.setItemVisible(id1, true);
    m.setItemEnabled(id1, false);
    m.setIgnoreDisabledActivation(true);
    CHECK_FALSE(m.activate(id1));
    CHECK(act == 0);

    // disabled allowed when ignoreDisabledActivation=false
    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id1));
    CHECK(act == 1);
}

TEST_CASE("Menu duplicate key updates existing item", "[gui][menu]")
{
    Menu m;
    int callsA = 0, callsB = 0;

    auto id1 = m.addItem("k", "Label1", [&]{ callsA++; }, true, true);
    auto id2 = m.addItem("k", "Label2", [&]{ callsB++; }, false, false);

    CHECK(id1 == id2);
    REQUIRE(m.items().size() == 1);

    const auto* it = m.getItem(id1);
    REQUIRE(it != nullptr);
    CHECK(it->label == "Label2");
    CHECK_FALSE(it->enabled);
    CHECK_FALSE(it->visible);

    // make visible, allow disabled activation, updated action used
    m.setItemVisible(id1, true);
    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id1));
    CHECK(callsA == 0);
    CHECK(callsB == 1);
}