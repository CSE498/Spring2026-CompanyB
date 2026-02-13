// Menu.cpp
// Group 21 GUI
//
// Implement the logic for the Menu (store items, select items, handle input)

#include "Menu.hpp"

#include <algorithm>
#include <utility> 

// default constructor
Menu::Menu()
{
}

// title constructor
Menu::Menu(std::string title)
    : m_title(std::move(title))
{
}

// item creation (add items)
// add item with key, label, and action
Menu::ItemId Menu::addItem(std::string key,
                           std::string label,
                           Action onActivate,
                           bool enabled,
                           bool visible)
{
}

// remove / clear
// delete items and update the key to id map
bool Menu::removeItem(ItemId id)
{
    // auto it = std::find_if(m_items.begin(), m_items.end(), [id](const Item& i) 
    // { return i.id == id; });

    // if (it == m_items.end())
    // return false

    //remove the key mapping for this item
    // m_keyToId.erase(it->key);
    
    //remove from list
    // m_items.erase(it)

    // fix selection if removed selected item
    // normalizeSelection();
    // return true;
}

// 
void Menu::clear()
{
    m_items.clear();
    m_keyToId.clear(); // keep m_nextId increasing so no id repeat
}

// get items
Menu::Item* Menu::getItem(ItemId id)
{
    auto it = std::find_if(m_items.begin(), m_items.end(),
            [id](const Item& i) { return i.id == id; });
    return (it == m_items.end()) ? nullptr : &(*it);
}

const std::vector<Menu::Item>& Menu::items() const
{
    return m_items;
}


// setters (dynamic ideas)
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


// helper functions
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

void Menu::normalizeSelection()
{
    // ifselected item exists and still selectable, keep
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

Menu::ItemId Menu::nextId()
{
    return m_nextId++; // move on to next id, no repeat
}


// seelection and navigation
bool Menu::select(ItemId id)
{
    bool found = false;

    for (auto& it : m_items) {
        if (it.id == id && isSelectable(it)) {
            // only one selected at a time
            it.selected = true;
            found = true;

            if (it.onSelected) it.onSelected();
        } else {
            it.selected = false;
        }
    }

    return found;
}

bool Menu::selectFirst()
{
    // for (auto& it : m_items) {
    //     if (isSelectable(it)) {
    //         return select(it.id);
    //     }
    // }
    // return false;
}

bool Menu::selectLast()
{
    // for (int i = static_cast<int>(m_items.size()) - 1; i >= 0; --i) {
    //     if (isSelectable(m_items[i])) {
    //         return select(m_items[i].id);
    //     }
    // }
    // return false;
}

bool Menu::selectNext()
{
    // work this out:
    // if (m_items.empty()) return false;
    // int selectedIndex = -1;
    // for (int i =0, i < static_cast<int>(m_items.size()); i++)
    // if statement if selected index = i, break
    // int start = selected index ? selected index + 1 : 0
    // return select m items[idx].id
    // return false
}

bool Menu::selectPrevious()
{
    // if m items.empty, return false
    // for int i = 0, i < static_cast<int> mitemsize ++i
    // if m items[i] selected, selected index = i then break
    // reverse order than select next
    // work this out
}

void Menu::hover(ItemId id)
{
    // Item* item = getItem(id);
    // if (!item) return;
    // if (item->onHover) item->onHover();
}


// activation
bool Menu::activateSelected()
{
    // for (auto& it : m_items) {
    //     if (it.selected) {
    //         return activate(it.id);
    //     }
    // }
    // return false;
}

bool Menu::activate(ItemId id)
{
    // Item* item = getItem(id);
    // if (!item) return false;

    // // if item invisible, ignore activation
    // if (!isVisible(*item)) return false;

    // // if disabled and ignoring disabled activation, nothing
    // if (!isEnabled(*item) && m_ignoreDisabledActivation) return false;
    // if (item->onActivate) {
    //     item->onActivate();
    //     return true;
    // }
    // return false;
}


// input handling
Menu::InputResult Menu::handleNav(NavEvent event)
{
    if (!m_open) return InputResult::Ignored;

    // switch (event): {

    // case NavEvent::Up:
    // case NavEvent::Left:
    // selectPrevious();
    // return InputResult::Heard;

    // do the rest:
    // NavEvent::Down, Right, Home, End, Activate, Back,

    // return InputResult::Heard;
    // } // end of switch 

    // return InputResult::Ignored;
}


// render model:
// loops over real stored items m_items
// computes the isVisible and isEnabled 
// returns a std::vector<RenderItem> for Gui to render
std::vector<Menu::RenderItem> Menu::buildRenderModel() const
{
    // std::vector<RenderItem> out; // list returned to GUI
    // out.reserve(m_items.size()); // avoid resizing

    // for (const auto& it : m_items) {
    //     RenderItem r;
    //     r.id = it.id; // keep id so gui can ref it
    //     r.label = it.label; // text to draw
    //     r.visible = isVisible(it); // final visibility
    //     r.enabled = isEnabled(it); // final enabled state
    //     r.selected = it.selected;  // highlight item if selected
    //     out.push_back(std::move(r)); // add to output list
    // }

    // return out;
}


// Menu structure
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
