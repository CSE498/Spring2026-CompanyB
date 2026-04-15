#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "Interfaces/gui/Menu.hpp"

using cse498::Menu;

static const Menu::RenderItem* findRenderItem(
    const std::vector<Menu::RenderItem>& items, Menu::ItemId id) {
  for (const auto& item : items) {
    if (item.id == id) {
      return &item;
    }
  }
  return nullptr;
}

TEST_CASE("Menu addItem returns stable ids and updates existing key",
          "[gui][menu]") {
  Menu m("Main Menu");

  int activated = 0;

  auto id1 = m.addItem("play", "Play", [&] { activated++; }, true, true);
  auto id2 = m.addItem("quit", "Quit", [] {}, true, true);

  REQUIRE(id1 != id2);

  auto id1Again =
      m.addItem("play", "Start Game", [&] { activated += 10; }, true, true);
  CHECK(id1Again == id1);

  auto model = m.buildRenderModel();
  REQUIRE(model.size() == 2);

  auto* playItem = findRenderItem(model, id1);
  REQUIRE(playItem != nullptr);
  CHECK(playItem->label == "Start Game");

  auto selectResult = m.select(id1);
  CHECK(selectResult.has_value());

  auto activateResult = m.activate(id1);
  CHECK(activateResult.has_value());
  CHECK(activated == 10);
}

TEST_CASE("Menu removeItem by id removes item and normalizes selection",
          "[gui][menu]") {
  Menu m;

  auto id1 = m.addItem("a", "A", [] {}, true, true);
  auto id2 = m.addItem("b", "B", [] {}, true, true);
  auto id3 = m.addItem("c", "C", [] {}, true, true);

  REQUIRE(m.select(id2).has_value());
  auto removeResult = m.removeItem(id2);
  CHECK(removeResult.has_value());

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

TEST_CASE("Menu removeItem by key removes matching item", "[gui][menu]") {
  Menu m;

  auto id1 = m.addItem("settings", "Settings", [] {}, true, true);
  auto id2 = m.addItem("help", "Help", [] {}, true, true);

  auto removeSettings = m.removeItem("settings");
  CHECK(removeSettings.has_value());

  auto removeMissing = m.removeItem("missing");
  CHECK_FALSE(removeMissing.has_value());
  CHECK(removeMissing.error() == "removeItem failed: key not found");

  auto model = m.buildRenderModel();
  REQUIRE(model.size() == 1);

  CHECK(findRenderItem(model, id1) == nullptr);
  CHECK(findRenderItem(model, id2) != nullptr);
}

TEST_CASE(
    "Menu selectFirst selectLast selectNext selectPrevious navigate selectable "
    "items only",
    "[gui][menu]") {
  Menu m;

  auto id1 = m.addItem("a", "A", [] {}, true, true);
  auto id2 = m.addItem("b", "B", [] {}, false, true);
  auto id3 = m.addItem("c", "C", [] {}, true, true);
  auto id4 = m.addItem("d", "D", [] {}, true, false);

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

TEST_CASE("Menu select returns error for disabled or invisible items",
          "[gui][menu]") {
  Menu m;

  auto disabledId = m.addItem("disabled", "Disabled", [] {}, false, true);
  auto hiddenId = m.addItem("hidden", "Hidden", [] {}, true, false);

  auto disabledResult = m.select(disabledId);
  CHECK_FALSE(disabledResult.has_value());
  CHECK(disabledResult.error() == "select failed: item is not selectable");

  auto hiddenResult = m.select(hiddenId);
  CHECK_FALSE(hiddenResult.has_value());
  CHECK(hiddenResult.error() == "select failed: item is not selectable");

  auto model = m.buildRenderModel();
  for (const auto& item : model) {
    CHECK_FALSE(item.selected);
  }
}

TEST_CASE("Menu select returns error for invalid id", "[gui][menu]") {
  Menu m;
  m.addItem("a", "A", [] {}, true, true);

  auto result = m.select(9999);
  CHECK_FALSE(result.has_value());
  CHECK(result.error() == "select failed: invalid item id");
}

TEST_CASE("Menu predicates affect visibility and enabled state",
          "[gui][menu]") {
  Menu m;

  bool allowEnabled = false;
  bool allowVisible = false;

  auto id = m.addItem("dynamic", "Dynamic", [] {}, true, true);

  CHECK(m.setEnabledPredicate(id, [&] { return allowEnabled; }).has_value());
  CHECK(m.setVisiblePredicate(id, [&] { return allowVisible; }).has_value());

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

TEST_CASE(
    "Menu activate returns error for disabled item when disabled activation is "
    "ignored",
    "[gui][menu]") {
  Menu m;

  int count = 0;
  auto id = m.addItem("x", "X", [&] { count++; }, false, true);

  auto result = m.activate(id);
  CHECK_FALSE(result.has_value());
  CHECK(result.error() == "activate failed: item is disabled");
  CHECK(count == 0);
}

TEST_CASE(
    "Menu activate succeeds for disabled item when ignoreDisabledActivation is "
    "false",
    "[gui][menu]") {
  Menu m;

  int count = 0;
  auto id = m.addItem("x", "X", [&] { count++; }, false, true);

  m.setIgnoreDisabledActivation(false);

  auto result = m.activate(id);
  CHECK(result.has_value());
  CHECK(count == 1);
}

TEST_CASE("Menu invisible items never activate", "[gui][menu]") {
  Menu m;

  int count = 0;
  auto id = m.addItem("x", "X", [&] { count++; }, true, false);

  auto result = m.activate(id);
  CHECK_FALSE(result.has_value());
  CHECK(result.error() == "activate failed: item is not visible");
  CHECK(count == 0);
}

TEST_CASE("Menu activate returns error when callback is missing",
          "[gui][menu]") {
  Menu m;

  auto id = m.addItem("x", "X", [] {}, true, true);
  CHECK(m.setActivateAction(id, Menu::Action{}).has_value());

  auto result = m.activate(id);
  CHECK_FALSE(result.has_value());
  CHECK(result.error() == "activate failed: item has no activation callback");
}

TEST_CASE("Menu hover works for visible items even if disabled",
          "[gui][menu]") {
  Menu m;

  int hoverCount = 0;
  auto id = m.addItem("x", "X", [] {}, false, true);
  CHECK(m.setHoverAction(id, [&] { hoverCount++; }).has_value());

  CHECK(m.hover(id));
  CHECK(hoverCount == 1);

  CHECK(m.setItemVisible(id, false).has_value());
  CHECK_FALSE(m.hover(id));
  CHECK(hoverCount == 1);
}

TEST_CASE("Menu onSelected callback fires only when selection changes",
          "[gui][menu]") {
  Menu m;

  int selectedA = 0;
  int selectedB = 0;

  auto id1 = m.addItem("a", "A", [] {}, true, true);
  auto id2 = m.addItem("b", "B", [] {}, true, true);

  CHECK(m.setSelectedAction(id1, [&] { selectedA++; }).has_value());
  CHECK(m.setSelectedAction(id2, [&] { selectedB++; }).has_value());

  auto toB = m.select(id2);
  CHECK(toB.has_value());
  CHECK(selectedA == 0);
  CHECK(selectedB == 1);

  auto toA = m.select(id1);
  CHECK(toA.has_value());
  CHECK(selectedA == 1);
  CHECK(selectedB == 1);

  auto toAAgain = m.select(id1);
  CHECK(toAAgain.has_value());
  CHECK(selectedA == 1);
  CHECK(selectedB == 1);

  auto backToB = m.select(id2);
  CHECK(backToB.has_value());
  CHECK(selectedA == 1);
  CHECK(selectedB == 2);
}

TEST_CASE(
    "Menu handleNav ignores input when closed and handles input when open",
    "[gui][menu]") {
  Menu m;

  int countA = 0;
  int countB = 0;

  auto id1 = m.addItem("a", "A", [&] { countA++; }, true, true);
  auto id2 = m.addItem("b", "B", [&] { countB++; }, true, true);

  REQUIRE(m.select(id1).has_value());

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

TEST_CASE("Menu clear removes all items", "[gui][menu]") {
  Menu m;

  m.addItem("a", "A", [] {}, true, true);
  m.addItem("b", "B", [] {}, true, true);

  REQUIRE_FALSE(m.buildRenderModel().empty());

  m.clear();

  CHECK(m.buildRenderModel().empty());
  CHECK_FALSE(m.findItemIdByKey("a").has_value());
  CHECK_FALSE(m.findItemIdByKey("b").has_value());
}

TEST_CASE("Menu title getters and setters work", "[gui][menu]") {
  Menu m;
  CHECK(m.title() == "");

  m.setTitle("Pause Menu");
  CHECK(m.title() == "Pause Menu");
}