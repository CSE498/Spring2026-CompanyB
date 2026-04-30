/**
 * This file implements the Menu class logic (item storage, selection,
 * navigation, predicate evaluation, and callback)
 */

#include "Menu.hpp"

#include <algorithm>
#include <utility>

namespace cse498 {

/**
 * Constructs a menu with a title.
 * @param title Menu title displayed by the GUI.
 */
Menu::Menu(std::string title) : m_title(std::move(title)) {}

/**
 * Adds a new item or updates an existing item with the same key.
 */
Menu::ItemId Menu::addItem(const std::string& key, const std::string& label,
                           Action onActivate, bool enabled, bool visible) {
  assert(!key.empty() && "addItem requires non empty key");

  auto itKey = m_keyToId.find(key);
  if (itKey != m_keyToId.end()) {
    Item* existing = getItem(itKey->second);
    assert(existing && "m_keyToId not synced with m_items");

    existing->label = label;
    existing->onActivate = std::move(onActivate);
    existing->enabled = enabled;
    existing->visible = visible;

    normalizeSelection();
    return existing->id;
  }

  Item item{};
  item.id = nextId();
  item.key = key;
  item.label = label;
  item.onActivate = std::move(onActivate);
  item.enabled = enabled;
  item.visible = visible;
  item.selected = false;

  m_keyToId[item.key] = item.id;
  m_items.push_back(std::move(item));

  normalizeSelection();
  return m_items.back().id;
}

/**
 * Removes an item by ItemId.
 * Updates storage and key->id mapping, then normalizes selection.
 */
std::expected<void, std::string> Menu::removeItem(ItemId id) {
  auto item = std::find_if(m_items.begin(), m_items.end(),
                           [id](const Item& it) { return it.id == id; });

  if (item == m_items.end()) {
    return std::unexpected("removeItem failed: invalid item id");
  }

  if (!item->key.empty()) {
    auto keyIt = m_keyToId.find(item->key);
    if (keyIt != m_keyToId.end() && keyIt->second == id) {
      m_keyToId.erase(keyIt);
    }
  }

  m_items.erase(item);
  normalizeSelection();
  return {};
}

/**
 * Removes an item by its string key.
 */
std::expected<void, std::string> Menu::removeItem(std::string_view key) {
  auto opt = findItemIdByKey(key);
  if (!opt) {
    return std::unexpected("removeItem failed: key not found");
  }

  return removeItem(*opt);
}

/**
 * Removes all items and clears the key->id mapping.
 */
void Menu::clear() {
  m_items.clear();
  m_keyToId.clear();
}

/**
 * Finds an ItemId by item key.
 */
std::optional<Menu::ItemId> Menu::findItemIdByKey(std::string_view key) const {
  auto it = m_keyToId.find(std::string(key));
  if (it == m_keyToId.end()) {
    return std::nullopt;
  }
  return it->second;
}

/**
 * Retrieves a mutable pointer to an item by ItemId.
 */
Menu::Item* Menu::getItem(ItemId id) {
  auto it = std::find_if(m_items.begin(), m_items.end(),
                         [id](const Item& item) { return item.id == id; });

  return (it == m_items.end()) ? nullptr : &(*it);
}

/**
 * Retrieves a const pointer to an item by ItemId.
 */
const Menu::Item* Menu::getItem(ItemId id) const {
  auto it = std::find_if(m_items.begin(), m_items.end(),
                         [id](const Item& item) { return item.id == id; });

  return (it == m_items.end()) ? nullptr : &(*it);
}

/**
 * Retrieves a mutable pointer to an item by key.
 */
Menu::Item* Menu::getItemByKey(std::string_view key) {
  auto opt = findItemIdByKey(key);
  return opt ? getItem(*opt) : nullptr;
}

/**
 * Retrieves a const pointer to an item by key.
 */
const Menu::Item* Menu::getItemByKey(std::string_view key) const {
  auto opt = findItemIdByKey(key);
  return opt ? getItem(*opt) : nullptr;
}

/**
 * Set an item's base enabled flag.
 */
std::expected<void, std::string> Menu::setItemEnabled(ItemId id, bool enabled) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setItemEnabled failed: invalid item id");
  }

  item->enabled = enabled;
  normalizeSelection();
  return {};
}

/**
 * Set an item's base visible flag.
 */
std::expected<void, std::string> Menu::setItemVisible(ItemId id, bool visible) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setItemVisible failed: invalid item id");
  }

  item->visible = visible;
  normalizeSelection();
  return {};
}

/**
 * Set an item's label text.
 */
std::expected<void, std::string> Menu::setItemLabel(ItemId id,
                                                    std::string label) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setItemLabel failed: invalid item id");
  }

  item->label = std::move(label);
  return {};
}

/**
 * Set an item's enabled predicate.
 */
std::expected<void, std::string> Menu::setEnabledPredicate(ItemId id,
                                                           Predicate pred) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setEnabledPredicate failed: invalid item id");
  }

  item->enabledIf = std::move(pred);
  normalizeSelection();
  return {};
}

/**
 * Set an item's visible predicate.
 */
std::expected<void, std::string> Menu::setVisiblePredicate(ItemId id,
                                                           Predicate pred) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setVisiblePredicate failed: invalid item id");
  }

  item->visibleIf = std::move(pred);
  normalizeSelection();
  return {};
}

/**
 * Set an item's activation callback.
 */
std::expected<void, std::string> Menu::setActivateAction(ItemId id,
                                                         Action action) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setActivateAction failed: invalid item id");
  }

  item->onActivate = std::move(action);
  return {};
}

/**
 * Set an item's hover action.
 */
std::expected<void, std::string> Menu::setHoverAction(ItemId id,
                                                      Action action) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setHoverAction failed: invalid item id");
  }

  item->onHover = std::move(action);
  return {};
}

/**
 * Set an item's selection callback.
 */
std::expected<void, std::string> Menu::setSelectedAction(ItemId id,
                                                         Action action) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("setSelectedAction failed: invalid item id");
  }

  item->onSelected = std::move(action);
  return {};
}

/**
 * Compute final visibility.
 */
bool Menu::isVisible(const Item& item) const {
  if (!item.visible) return false;
  if (item.visibleIf) return item.visibleIf();
  return true;
}

/**
 * Compute final enabled state.
 */
bool Menu::isEnabled(const Item& item) const {
  if (!item.enabled) return false;
  if (item.enabledIf) return item.enabledIf();
  return true;
}

/**
 * Compute whether item can be selected.
 */
bool Menu::isSelectable(const Item& item) const {
  return isVisible(item) && isEnabled(item);
}

/**
 * Repair selection to ensure a valid selectable item is selected.
 */
void Menu::normalizeSelection() {
  for (auto& item : m_items) {
    if (item.selected) {
      if (isSelectable(item)) return;
      item.selected = false;
      break;
    }
  }

  selectFirst();
}

/**
 * Generate next unique ItemId.
 */
Menu::ItemId Menu::nextId() { return m_nextId++; }

/**
 * Select an item by ItemId.
 */
std::expected<void, std::string> Menu::select(ItemId id) {
  Item* target = getItem(id);
  if (!target) {
    return std::unexpected("select failed: invalid item id");
  }

  if (!isSelectable(*target)) {
    for (auto& item : m_items) {
      item.selected = false;
    }
    return std::unexpected("select failed: item is not selectable");
  }

  const bool wasAlreadySelected = target->selected;

  for (auto& item : m_items) {
    item.selected = (&item == target);
  }

  if (!wasAlreadySelected && target->onSelected) {
    target->onSelected();
  }

  return {};
}

/**
 * Select the first selectable item.
 */
bool Menu::selectFirst() {
  auto it =
      std::find_if(m_items.begin(), m_items.end(),
                   [this](const Item& item) { return isSelectable(item); });

  if (it == m_items.end()) return false;
  return select(it->id).has_value();
}

/**
 * Select the last selectable item.
 */
bool Menu::selectLast() {
  auto it =
      std::find_if(m_items.rbegin(), m_items.rend(),
                   [this](const Item& item) { return isSelectable(item); });

  if (it == m_items.rend()) return false;
  return select(it->id).has_value();
}

/**
 * Select the next selectable item.
 */
bool Menu::selectNext() {
  if (m_items.empty()) return false;

  std::optional<std::size_t> selectedIndex;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    if (m_items[i].selected) {
      selectedIndex = i;
      break;
    }
  }

  const std::size_t n = m_items.size();
  std::size_t start = selectedIndex ? ((*selectedIndex + 1) % n) : 0;

  for (std::size_t step = 0; step < n; ++step) {
    const std::size_t idx = (start + step) % n;
    if (isSelectable(m_items[idx])) {
      return select(m_items[idx].id).has_value();
    }
  }

  return false;
}

/**
 * Select the previous selectable item.
 */
bool Menu::selectPrevious() {
  if (m_items.empty()) return false;

  std::optional<std::size_t> selectedIndex;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    if (m_items[i].selected) {
      selectedIndex = i;
      break;
    }
  }

  const std::size_t n = m_items.size();
  std::size_t start = selectedIndex ? ((*selectedIndex + n - 1) % n) : (n - 1);

  for (std::size_t step = 0; step < n; ++step) {
    const std::size_t idx = (start + n - step) % n;
    if (isSelectable(m_items[idx])) {
      return select(m_items[idx].id).has_value();
    }
  }

  return false;
}

/**
 * Hover an item by ItemId.
 */
bool Menu::hover(ItemId id) {
  Item* item = getItem(id);
  if (!item) return false;

  if (!isVisible(*item)) return false;

  if (item->onHover) {
    item->onHover();
  }
  return true;
}

/**
 * Activate the currently selected item.
 */
std::expected<void, std::string> Menu::activateSelected() {
  auto it = std::find_if(m_items.begin(), m_items.end(),
                         [](const Item& item) { return item.selected; });

  if (it == m_items.end()) {
    return std::unexpected("activateSelected failed: no selected item");
  }

  return activate(it->id);
}

/**
 * Activate an item by ItemId.
 */
std::expected<void, std::string> Menu::activate(ItemId id) {
  Item* item = getItem(id);
  if (!item) {
    return std::unexpected("activate failed: invalid item id");
  }

  if (!isVisible(*item)) {
    return std::unexpected("activate failed: item is not visible");
  }

  if (!isEnabled(*item) && m_ignoreDisabledActivation) {
    return std::unexpected("activate failed: item is disabled");
  }

  if (!item->onActivate) {
    return std::unexpected("activate failed: item has no activation callback");
  }

  item->onActivate();
  return {};
}

/**
 * Handle a navigation event.
 */
Menu::InputResult Menu::handleNav(NavEvent event) {
  if (!m_open) return InputResult::Ignored;

  switch (event) {
    case NavEvent::Up:
    case NavEvent::Left:
      selectPrevious();
      return InputResult::Heard;

    case NavEvent::Down:
    case NavEvent::Right:
      selectNext();
      return InputResult::Heard;

    case NavEvent::Home:
      selectFirst();
      return InputResult::Heard;

    case NavEvent::End:
      selectLast();
      return InputResult::Heard;

    case NavEvent::Activate:
      activateSelected();
      return InputResult::Heard;

    case NavEvent::Back:
      m_open = false;
      return InputResult::Heard;
  }

  return InputResult::Ignored;
}

/**
 * Build a render snapshot for the GUI.
 */
std::vector<Menu::RenderItem> Menu::buildRenderModel() const {
  std::vector<RenderItem> renderItems;
  renderItems.reserve(m_items.size());

  for (const auto& item : m_items) {
    RenderItem r;
    r.id = item.id;
    r.label = item.label;
    r.visible = isVisible(item);
    r.enabled = isEnabled(item);
    r.selected = item.selected;
    renderItems.push_back(std::move(r));
  }

  return renderItems;
}

/**
 * Set the menu title.
 */
void Menu::setTitle(std::string title) { m_title = std::move(title); }

/**
 * Get the menu title.
 */
const std::string& Menu::title() const { return m_title; }

/**
 * Set whether the menu is open.
 */
void Menu::setOpen(bool open) { m_open = open; }

/**
 * Get whether the menu is open.
 */
bool Menu::isOpen() const { return m_open; }

/**
 * Set whether disabled items are blocked from activation.
 */
void Menu::setIgnoreDisabledActivation(bool ignore) {
  m_ignoreDisabledActivation = ignore;
}

/**
 * Get whether disabled items are blocked from activation.
 */
bool Menu::ignoreDisabledActivation() const {
  return m_ignoreDisabledActivation;
}

}  // namespace cse498