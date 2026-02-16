// Menu.cpp
// Group 21 GUI
//
// Implement the logic for the Menu (store items, select items, handle input)

#include "Menu.hpp"

#include <algorithm>
#include <utility>

namespace cse498 {

// default constructor
Menu::Menu() = default;

// title constructor
Menu::Menu(std::string title)
    : m_title(std::move(title))
{
}

// add an item to the menu
// key is for code lookups, label is what user sees
Menu::ItemId Menu::addItem(std::string key,
                           std::string label,
                           Action onActivate,
                           bool enabled,
                           bool visible)
{
    // key should never be empty
    assert(!key.empty() && "Menu::addItem requires non empty key");

    // if key already exists, update existing item
    // keeps menu dynamic and prevents duplicate keys
    if (auto itKey = m_keyToId.find(key); itKey != m_keyToId.end()) {
        Item* existing = getItem(itKey->second);
        assert(existing && "m_keyToId not synced with m_items");

        existing->label = std::move(label);
        existing->onActivate = std::move(onActivate);
        existing->enabled = enabled;
        existing->visible = visible;

        // selection may move if change enable/visible
        normalizeSelection();
        return existing->id;
    }

    // new item
    Item item;
    item.id = nextId();
    item.key = std::move(key);
    item.label = std::move(label);
    item.onActivate = std::move(onActivate);
    item.enabled = enabled;
    item.visible = visible;
    item.selected = false;

    // store mapping so code can find items by key
    m_keyToId[item.key] = item.id;
    m_items.push_back(std::move(item));

    // ensure selection if possible
    normalizeSelection();
    return m_items.back().id;
}

// remove / clear
// delete items and update the key to id map
bool Menu::removeItem(ItemId id)
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
            [id](const Item& i) { return i.id == id; });
    if (it == m_items.end()) {
        return false;
    }

    // remove the key mapping if matches the id
    if (!it->key.empty()) {
        auto itKey = m_keyToId.find(it->key);
        if (itKey != m_keyToId.end() && itKey->second == id) {
            m_keyToId.erase(itKey);
        }
    }

    const bool removedWasSelected = it->selected;

    m_items.erase(it);

    // if selected item was removed selection needs repair
    // also covers case where predicates changed and selection became invalid
    if (removedWasSelected) normalizeSelection();
    else normalizeSelection();
    return true;
}

// remove everything
void Menu::clear()
{
    m_items.clear();
    m_keyToId.clear(); 
    // keep m_nextId increasing so no id repeat
}

// get mutable items by id
Menu::Item* Menu::getItem(ItemId id)
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
            [id](const Item& i) { return i.id == id; });
    return (it == m_items.end()) ? nullptr : &(*it);
}

// for const correctness (overload)
// want to be able to look up items that does not modify menu
// get read only item by id
// allows calling from const functions like buildrendermodel
const Menu::Item* Menu::getItem(ItemId id) const
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
            [id](const Item& i) { return i.id == id; });
    return (it == m_items.end()) ? nullptr : &(*it);
}

// read only list access
const std::vector<Menu::Item>& Menu::items() const
{
    return m_items;
}


// setters (dynamic ideas)
// normalize selection after enabled or visible changes
void Menu::setItemEnabled(ItemId id, bool enabled)
{
    Item* item = getItem(id);
    if (!item) return;
    item->enabled = enabled;
    normalizeSelection();
}

void Menu::setItemVisible(ItemId id, bool visible)
{
    Item* item = getItem(id);
    if (!item) return;
    item->visible = visible;
    normalizeSelection();
}

void Menu::setItemLabel(ItemId id, std::string label)
{
    Item* item = getItem(id);
    if (!item) return;
    item->label = std::move(label);
}

void Menu::setEnabledPredicate(ItemId id, Predicate pred)
{
    Item* item = getItem(id);
    if (!item) return;
    item->enabledIf = std::move(pred);
    normalizeSelection();
}

void Menu::setVisiblePredicate(ItemId id, Predicate pred)
{
    Item* item = getItem(id);
    if (!item) return;
    item->visibleIf = std::move(pred);
    normalizeSelection();
}

void Menu::setActivateAction(ItemId id, Action action)
{
    Item* item = getItem(id);
    if (!item) return;
    item->onActivate = std::move(action);
}

void Menu::setHoverAction(ItemId id, Action action)
{
    Item* item = getItem(id);
    if (!item) return;
    item->onHover = std::move(action);
}

void Menu::setSelectedAction(ItemId id, Action action)
{
    Item* item = getItem(id);
    if (!item) return;
    item->onSelected = std::move(action);
}


// helper functions (predicate helpers)
bool Menu::isVisible(const Item& item) const
{
    if (!item.visible) return false;
    if (item.visibleIf) return item.visibleIf();
    return true;
}

bool Menu::isEnabled(const Item& item) const
{
    if (!item.enabled) return false;
    if (item.enabledIf) return item.enabledIf();
    return true;
}

bool Menu::isSelectable(const Item& item) const
{
    // // selectable mean cursor can land on it
    // // require visible and enabled
    return isVisible(item) && isEnabled(item);
}

// ensure exactly one valid selected item if possible
void Menu::normalizeSelection()
{
    // if current selection still ok keep it
    for (auto& it : m_items) {
        if (it.selected) {
            if (isSelectable(it)) return;
            it.selected = false;
            break;
        }
    }

    // otherwise pick the first select item
    selectFirst();
}

// generate unique ids
Menu::ItemId Menu::nextId()
{
    return m_nextId++; // move on to next id, no repeat
}


// selection and navigation - select by id
bool Menu::select(ItemId id)
{
    bool found = false;

    // ensure only one selected at a time
    for (auto& it : m_items) {
        const bool shouldSelect = (it.id == id) && isSelectable(it);
        if (shouldSelect) {
            // trigger selected callback only on transition
            if (!it.selected) {
                it.selected = true;
                if (it.onSelected) it.onSelected();
            } else {
                it.selected = true;
            }
            found = true;
        } else {
            it.selected = false;
        }
    }

    return found;
}

// finds first selectable item that is
// visible + enabled and marks it selected
// used when menu opens or when current selection become invalid
bool Menu::selectFirst()
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
        [this](const Item& item) { return isSelectable(item); });
    if (it == m_items.end()) return false;
    return select(it->id);
}

// find last selectable item and select it
// 'go to bottom' behavior/scroll to last option
bool Menu::selectLast()
{
    auto it = std::find_if(m_items.rbegin(), m_items.rend(),
        [this](const Item& item) { return isSelectable(item); });
    if (it == m_items.rend()) return false;
    return select(it->id);
}

// move selection down to next selectale item
// skip items disabled/hidden
// wrap around to top if at end
// used for down arrow key
bool Menu::selectNext()
{
    if (m_items.empty()) return false;

    // find current selection index
    std::optional<std::size_t> selectedIndex;
    for (std::size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].selected) {
            selectedIndex = i;
            break;
        }
    }

    // start searching after current selection
    const std::size_t n = m_items.size();
    std::size_t start = selectedIndex ? ((*selectedIndex + 1) % n) : 0;

    // wrap around and skip non selectable items
    for (std::size_t step = 0; step < n; ++step) {
        const std::size_t idx = (start + step) % n;
        if (isSelectable(m_items[idx])) {
            return select(m_items[idx].id);
        }
    }
    return false;
}

// move selection up to previous selectable item
// skip diabled and hidden, wrap around to bottom if at top
// used for up arrow
bool Menu::selectPrevious()
{
    if (m_items.empty()) return false;

    // find current selection index
    std::optional<std::size_t> selectedIndex;
    for (std::size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].selected) {
            selectedIndex = i;
            break;
        }
    }

    const std::size_t n = m_items.size();

    // start searching before current selection
    std::size_t start = selectedIndex ? ((*selectedIndex + n - 1) % n) : (n - 1);

    // wrap around and skip non selectable items
    for (std::size_t step = 0; step < n; ++step) {
        const std::size_t idx = (start + n - step) % n;
        if (isSelectable(m_items[idx])) {
            return select(m_items[idx].id);
        }
    }
    return false;
}

void Menu::hover(ItemId id)
{
    // hover is for ui effects like tooltips or sounds
    // allow hovering disabled items but not invisible items
    Item* item = getItem(id);
    if (!item) return; 
    if (!isVisible(*item)) return; 
    if (item->onHover) item->onHover();
}


// activation
bool Menu::activateSelected()
{
    // run action on currently selected item
    auto it = std::find_if(m_items.begin(), m_items.end(),
            [](const Item& item) { return item.selected; });
    if (it == m_items.end()) return false;
    return activate(it->id);
}

// 
bool Menu::activate(ItemId id)
{
    Item* item = getItem(id);
    if (!item) return false;

    // invisible items should never activate
    if (!isVisible(*item)) return false; 

    // if disabled and ignore action appears, then block item
    if (!isEnabled(*item) && m_ignoreDisabledActivation) {
        return false;
    }

    // otherwise callback, run and return true
    if (item->onActivate) {
        item->onActivate();
        return true;
    }
    return false;
}


// input handling
// up, left = previous
// down, right = next
// home = first
// end = last
// activate = activateSelected
// back = close menu (setOpen(false))
Menu::InputResult Menu::handleNav(NavEvent event)
{
    // if menu is closed ignore input
    if (!m_open) return InputResult::Ignored;

    // map events to selection or activation
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
        // close menu on back?
        m_open = false;
        return InputResult::Heard;
    }

    return InputResult::Ignored;
}


// render model:
// loops over real stored items m_items
// computes the isVisible and isEnabled 
// returns a std::vector<RenderItem> for Gui to render
// what the gui calls to draw
std::vector<Menu::RenderItem> Menu::buildRenderModel() const
{
    std::vector<RenderItem> out; // list returned to GUI
    out.reserve(m_items.size()); // avoid resizing

    for (const auto& it : m_items) {
        RenderItem r;
        r.id = it.id; // keep id so gui can ref it
        r.label = it.label; // text to draw
        r.visible = isVisible(it); // final visibility
        r.enabled = isEnabled(it); // final enabled state
        r.selected = it.selected;  // highlight item if selected
        out.push_back(std::move(r)); // add to output list
    }

    return out;
}


// Menu structure
// simple title getters setters 
void Menu::setTitle(std::string title)
{
    m_title = std::move(title);
}

const std::string& Menu::title() const
{
    return m_title;
}

void Menu::setOpen(bool open)
{
    m_open = open;
}

bool Menu::isOpen() const
{
    return m_open;
}

void Menu::setIgnoreDisabledActivation(bool ignore)
{
    m_ignoreDisabledActivation = ignore;
}

bool Menu::ignoreDisabledActivation() const
{
    return m_ignoreDisabledActivation;
}

} // namespce 498
