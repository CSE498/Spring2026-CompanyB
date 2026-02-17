// MenuTest.cpp

#include "../../source/Interfaces/gui/Menu.hpp"


#include <iostream>
#include <string>
#include <vector>

using cse498::Menu;

static int g_failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { ++g_failures; std::cerr << "FAIL: " << #expr << " (" << __FILE__ << ":" << __LINE__ << ")\n"; } \
} while (0)

#define CHECK_EQ(a,b) do { \
    auto lhs = (a); auto rhs = (b); \
    if (!(lhs == rhs)) { ++g_failures; std::cerr << "FAIL: " << #a << " == " << #b << " (" << __FILE__ << ":" << __LINE__ << ")\n"; } \
} while (0)



static void test_title_get_set()
{
    Menu m;
    CHECK(m.title().empty());

    m.setTitle("Main Menu");
    CHECK_EQ(m.title(), std::string("Main Menu"));

    Menu m2(std::string("Options"));
    CHECK_EQ(m2.title(), std::string("Options"));
}

static void test_open_get_set_and_handleNav_gate()
{
    Menu m;
    CHECK(m.isOpen()); // default true

    m.setOpen(false);
    CHECK(!m.isOpen());

    // Closed menu should ignore input
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Ignored);
    CHECK(m.handleNav(Menu::NavEvent::Back) == Menu::InputResult::Ignored);

    m.setOpen(true);
    CHECK(m.isOpen());
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Heard);
}

static void test_ignoreDisabledActivation_get_set()
{
    Menu m;
    CHECK(m.ignoreDisabledActivation()); // default true

    m.setIgnoreDisabledActivation(false);
    CHECK(!m.ignoreDisabledActivation());

    m.setIgnoreDisabledActivation(true);
    CHECK(m.ignoreDisabledActivation());
}

static void test_addItem_getItem_items()
{
    Menu m;
    bool activated = false;

    auto id = m.addItem("play", "Play", [&]{ activated = true; }, true, true);

    CHECK(id != 0);
    CHECK_EQ(m.items().size(), std::size_t(1));

    // mutable getItem
    Menu::Item* it = m.getItem(id);
    CHECK(it != nullptr);
    CHECK_EQ(it->key, std::string("play"));
    CHECK_EQ(it->label, std::string("Play"));
    CHECK(it->enabled);
    CHECK(it->visible);

    // const getItem
    const Menu& cm = m;
    const Menu::Item* cit = cm.getItem(id);
    CHECK(cit != nullptr);
    CHECK_EQ(cit->key, std::string("play"));
}

static void test_item_setters_enabled_visible_label()
{
    Menu m;
    auto id = m.addItem("x", "X", []{}, true, true);

    m.setItemLabel(id, "New Label");
    m.setItemEnabled(id, false);
    m.setItemVisible(id, false);

    const Menu::Item* it = m.getItem(id);
    CHECK(it != nullptr);
    CHECK_EQ(it->label, std::string("New Label"));
    CHECK(!it->enabled);
    CHECK(!it->visible);
}

static void test_predicates_affect_render_model()
{
    Menu m;
    auto id = m.addItem("x", "X", []{}, true, true);

    bool allowEnabled = true;
    bool allowVisible = true;

    m.setEnabledPredicate(id, [&]{ return allowEnabled; });
    m.setVisiblePredicate(id, [&]{ return allowVisible; });

    // both true
    {
        auto rm = m.buildRenderModel();
        CHECK_EQ(rm.size(), std::size_t(1));
        CHECK(rm[0].enabled);
        CHECK(rm[0].visible);
    }

    // enabled false
    allowEnabled = false;
    {
        auto rm = m.buildRenderModel();
        CHECK(!rm[0].enabled);
        CHECK(rm[0].visible);
    }

    // visible false
    allowVisible = false;
    {
        auto rm = m.buildRenderModel();
        CHECK(!rm[0].visible);
    }
}

static void test_action_setters_activate_hover_selected()
{
    Menu m;

    int activated = 0;
    int hovered = 0;
    int selected = 0;

    auto id1 = m.addItem("one", "One", []{}, true, true);
    auto id2 = m.addItem("two", "Two", []{}, true, true);

    m.setActivateAction(id1, [&]{ activated++; });
    m.setHoverAction(id1, [&]{ hovered++; });
    m.setSelectedAction(id2, [&]{ selected++; });

    // hover should run if visible
    m.hover(id1);
    CHECK_EQ(hovered, 1);

    // selecting triggers onSelected only on transition
    CHECK(m.select(id2));
    CHECK_EQ(selected, 1);

    CHECK(m.select(id2));
    CHECK_EQ(selected, 1); // no second transition

    // activation runs activate callback when allowed
    CHECK(m.activate(id1));
    CHECK_EQ(activated, 1);
}

static void test_activation_rules_visibility_and_disabled()
{
    Menu m;
    int act = 0;
    auto id = m.addItem("x", "X", [&]{ act++; }, true, true);

    // invisible never activates
    m.setItemVisible(id, false);
    CHECK(!m.activate(id));
    CHECK_EQ(act, 0);

    // visible again
    m.setItemVisible(id, true);

    // disabled + ignoreDisabledActivation=true blocks
    m.setItemEnabled(id, false);
    m.setIgnoreDisabledActivation(true);
    CHECK(!m.activate(id));
    CHECK_EQ(act, 0);

    // disabled + ignoreDisabledActivation=false allows
    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id));
    CHECK_EQ(act, 1);
}

static void test_addItem_sameKey_updates_existing()
{
    Menu m;
    int callsA = 0;
    int callsB = 0;

    auto id1 = m.addItem("k", "Label1", [&]{ callsA++; }, true, true);
    auto id2 = m.addItem("k", "Label2", [&]{ callsB++; }, false, false);

    CHECK_EQ(id1, id2);
    CHECK_EQ(m.items().size(), std::size_t(1));

    const Menu::Item* it = m.getItem(id1);
    CHECK(it != nullptr);
    CHECK_EQ(it->label, std::string("Label2"));
    CHECK(!it->enabled);
    CHECK(!it->visible);

    m.setItemVisible(id1, true);

    // allow disabled activation and verify updated action is used
    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id1));
    CHECK_EQ(callsA, 0);
    CHECK_EQ(callsB, 1);
}

static void test_remove_and_clear()
{
    Menu m;
    auto id1 = m.addItem("a", "A", []{}, true, true);
    auto id2 = m.addItem("b", "B", []{}, true, true);

    CHECK(m.removeItem(id1));
    CHECK(m.getItem(id1) == nullptr);
    CHECK(m.getItem(id2) != nullptr);

    CHECK(!m.removeItem(999999u)); // non existent

    m.clear();
    CHECK_EQ(m.items().size(), std::size_t(0));
}

int main()
{
    test_title_get_set();
    test_open_get_set_and_handleNav_gate();
    test_ignoreDisabledActivation_get_set();

    test_addItem_getItem_items();
    test_item_setters_enabled_visible_label();
    test_predicates_affect_render_model();

    test_action_setters_activate_hover_selected();
    test_activation_rules_visibility_and_disabled();

    test_addItem_sameKey_updates_existing();
    test_remove_and_clear();

    if (g_failures == 0) {
        std::cout << "[OK] All Menu tests passed.\n";
        return 0;
    }

    std::cerr << "[DONE] Failures: " << g_failures << "\n";
    return 1;
}
