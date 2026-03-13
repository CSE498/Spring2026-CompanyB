#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "Interfaces/gui/Menu.hpp"

using cse498::Menu;

static const Menu::RenderItem* findRenderItem(const std::vector<Menu::RenderItem>& items,
                                              Menu::ItemId id)
{
    for (const auto& item : items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

TEST_CASE("Menu addItem returns stable ids and updates existing key", "[gui][menu]")
{
    Menu m("Main Menu");

    int activated = 0;

    auto id1 = m.addItem("play", "Play", [&] { activated++; }, true, true);
    auto id2 = m.addItem("quit", "Quit", [] {}, true, true);

    REQUIRE(id1 != id2);

    auto id1_again = m.addItem("play", "Start Game", [&] { activated += 10; }, true, true);

    CHECK(id1_again == id1);

    auto model = m.buildRenderModel();
    REQUIRE(model.size() == 2);

    auto* playItem = findRenderItem(model, id1);
    REQUIRE(playItem != nullptr);
    CHECK(playItem->label == "Start Game");

    CHECK(m.select(id1));
    CHECK(m.activateSelected());
    CHECK(activated == 10);
}

TEST_CASE("Menu removeItem by id removes item and normalizes selection", "[gui][menu]")
{
    Menu m;

    auto id1 = m.addItem("a", "A", [] {}, true, true);
    auto id2 = m.addItem("b", "B", [] {}, true, true);
    auto id3 = m.addItem("c", "C", [] {}, true, true);

    REQUIRE(m.select(id2));

    CHECK(m.removeItem(id2));

    auto model = m.buildRenderModel();
    REQUIRE(model.size() == 2);

    CHECK(findRenderItem(model, id2) == nullptr);
    CHECK(findRenderItem(model, id1) != nullptr);
    CHECK(findRenderItem(model, id3) != nullptr);

    int selectedCount = 0;
    for (const auto& item : model) {
        if (item.selected) {
            selectedCount++;
        }
    }
    CHECK(selectedCount == 1);
}

TEST_CASE("Menu removeItem by key removes matching item", "[gui][menu]")
{
    Menu m;

    auto id1 = m.addItem("settings", "Settings", [] {}, true, true);
    auto id2 = m.addItem("help", "Help", [] {}, true, true);

    CHECK(m.removeItem("settings"));
    CHECK_FALSE(m.removeItem("missing"));

    auto model = m.buildRenderModel();
    REQUIRE(model.size() == 1);

    CHECK(findRenderItem(model, id1) == nullptr);
    CHECK(findRenderItem(model, id2) != nullptr);
}

TEST_CASE("Menu selectFirst selectLast selectNext selectPrevious navigate selectable items only", "[gui][menu]")
{
    Menu m;

    auto id1 = m.addItem("a", "A", [] {}, true, true);
    auto id2 = m.addItem("b", "B", [] {}, false, true); // disabled
    auto id3 = m.addItem("c", "C", [] {}, true, true);
    auto id4 = m.addItem("d", "D", [] {}, true, false); // invisible

    CHECK(m.selectFirst());

    auto model = m.buildRenderModel();
    REQUIRE(findRenderItem(model, id1) != nullptr);
    CHECK(findRenderItem(model, id1)->selected);

    CHECK(m.selectNext());
    model = m.buildRenderModel();
    CHECK(findRenderItem(model, id3)->selected);

    CHECK(m.selectNext());
    model = m.buildRenderModel();
    CHECK(findRenderItem(model, id1)->selected);

    CHECK(m.selectLast());
    model = m.buildRenderModel();
    CHECK(findRenderItem(model, id3)->selected);

    CHECK(m.selectPrevious());
    model = m.buildRenderModel();
    CHECK(findRenderItem(model, id1)->selected);

    (void)id2;
    (void)id4;
}

TEST_CASE("Menu select fails for disabled or invisible items", "[gui][menu]")
{
    Menu m;

    auto disabledId = m.addItem("disabled", "Disabled", [] {}, false, true);
    auto hiddenId = m.addItem("hidden", "Hidden", [] {}, true, false);

    CHECK_FALSE(m.select(disabledId));
    CHECK_FALSE(m.select(hiddenId));

    auto model = m.buildRenderModel();
    for (const auto& item : model) {
        CHECK_FALSE(item.selected);
    }
}

TEST_CASE("Menu predicates affect visibility and enabled state", "[gui][menu]")
{
    Menu m;

    bool allowEnabled = false;
    bool allowVisible = false;

    auto id = m.addItem("dynamic", "Dynamic", [] {}, true, true);

    m.setEnabledPredicate(id, [&] { return allowEnabled; });
    m.setVisiblePredicate(id, [&] { return allowVisible; });

    auto model = m.buildRenderModel();
    auto* item = findRenderItem(model, id);
    REQUIRE(item != nullptr);
    CHECK_FALSE(item->enabled);
    CHECK_FALSE(item->visible);

    allowEnabled = true;
    allowVisible = true;

    model = m.buildRenderModel();
    item = findRenderItem(model, id);
    REQUIRE(item != nullptr);
    CHECK(item->enabled);
    CHECK(item->visible);
}

TEST_CASE("Menu activation respects ignoreDisabledActivation", "[gui][menu]")
{
    Menu m;

    int count = 0;
    auto id = m.addItem("x", "X", [&] { count++; }, false, true);

    CHECK_FALSE(m.activate(id));
    CHECK(count == 0);

    m.setIgnoreDisabledActivation(false);
    CHECK(m.activate(id));
    CHECK(count == 1);
}

TEST_CASE("Menu invisible items never activate", "[gui][menu]")
{
    Menu m;

    int count = 0;
    auto id = m.addItem("x", "X", [&] { count++; }, true, false);

    CHECK_FALSE(m.activate(id));
    CHECK(count == 0);
}

TEST_CASE("Menu hover works for visible items even if disabled", "[gui][menu]")
{
    Menu m;

    int hoverCount = 0;
    auto id = m.addItem("x", "X", [] {}, false, true);
    m.setHoverAction(id, [&] { hoverCount++; });

    CHECK(m.hover(id));
    CHECK(hoverCount == 1);

    m.setItemVisible(id, false);
    CHECK_FALSE(m.hover(id));
    CHECK(hoverCount == 1);
}

TEST_CASE("Menu onSelected callback runs only when selection changes", "[gui][menu]")
{
    Menu m;

    int selectedA = 0;
    int selectedB = 0;

    auto id1 = m.addItem("a", "A", [] {}, true, true);
    auto id2 = m.addItem("b", "B", [] {}, true, true);

    m.setSelectedAction(id1, [&] { selectedA++; });
    m.setSelectedAction(id2, [&] { selectedB++; });

    CHECK(m.select(id2));
    CHECK(selectedA == 0);
    CHECK(selectedB == 1);

    CHECK(m.select(id1));
    CHECK(selectedA == 1);
    CHECK(selectedB == 1);

    CHECK(m.select(id1));
    CHECK(selectedA == 1);
    CHECK(selectedB == 1);

    CHECK(m.select(id2));
    CHECK(selectedA == 1);
    CHECK(selectedB == 2);
}

TEST_CASE("Menu handleNav ignores input when closed and handles input when open", "[gui][menu]")
{
    Menu m;

    int countA = 0;
    int countB = 0;

    auto id1 = m.addItem("a", "A", [&] { countA++; }, true, true);
    auto id2 = m.addItem("b", "B", [&] { countB++; }, true, true);

    REQUIRE(m.select(id1));

    m.setOpen(false);
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Ignored);

    auto model = m.buildRenderModel();
    CHECK(findRenderItem(model, id1)->selected);

    m.setOpen(true);
    CHECK(m.handleNav(Menu::NavEvent::Down) == Menu::InputResult::Heard);

    model = m.buildRenderModel();
    CHECK(findRenderItem(model, id2)->selected);

    CHECK(m.handleNav(Menu::NavEvent::Activate) == Menu::InputResult::Heard);
    CHECK(countB == 1);

    CHECK(m.handleNav(Menu::NavEvent::Back) == Menu::InputResult::Heard);
    CHECK_FALSE(m.isOpen());
}

TEST_CASE("Menu clear removes all items", "[gui][menu]")
{
    Menu m;

    m.addItem("a", "A", [] {}, true, true);
    m.addItem("b", "B", [] {}, true, true);

    REQUIRE_FALSE(m.buildRenderModel().empty());

    m.clear();

    CHECK(m.buildRenderModel().empty());
    CHECK_FALSE(m.findItemIdByKey("a").has_value());
    CHECK_FALSE(m.findItemIdByKey("b").has_value());
}

TEST_CASE("Menu title getters and setters work", "[gui][menu]")
{
    Menu m;
    CHECK(m.title() == "");

    m.setTitle("Pause Menu");
    CHECK(m.title() == "Pause Menu");
}