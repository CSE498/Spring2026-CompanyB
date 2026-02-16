// Menu header file
// Group 21 GUI Interface

/*
* Usage:
* - Build Menus dynamically (add, remove, enable, disable)
* - Navigate with keyboard/mouse/controller through input events
* - Trigger application actions (page navigation, toggle, debug commands)
*/

#ifndef MENU_HPP
#define MENU_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class Menu {
public:
    using ItemId = std::uint32_t;

    // callback type for menu actions
    using Action = std::function<void()>;

    // Predicate for dynamic enabled/visibility rules
    using Predicate = std::function<bool()>;

    // navigation events
    // translate keyboard mouse movements
    enum class NavEvent {
        Up, // arrow up would be NavEvent::Up
        Down,
        Left,
        Right,
        Activate,
        Back,
        Home,
        End
    };

    // menu responds to input
    enum class InputResult {
        Ignored, // if menu closed, ignore input
        Heard // if menu open, selection made, result is heard
    };

    // render item representation
    struct RenderItem { // not the real stored menu item
        ItemId id{};    // menu = logic, render = visuals
        std::string label{};
        bool enabled{true};
        bool visible{true};
        bool selected{false};
        //std::string tooltip{};
    };

    // menu item representation
    // items can be added at runtime
    struct Item {
        ItemId id{};
        std::string key;       // programmer's side
        std::string label;     // user's side
        //std::string tooltip; // optional help text? can add later if needed, removed for now

        bool enabled{true};     // basic flags
        bool visible{true};
        bool selected{false}; // whether item is currently selected 

        // runtime conditions?? (optional)
        Predicate enabledIf{};
        Predicate visibleIf{};

        // callback
        Action onActivate{};
        Action onHover{};
        Action onSelected{};
    };

    // creating menu
    Menu();
    explicit Menu(std::string title);

    // manage items
    ItemId addItem(std::string key,
                   std::string label,
                   Action onActivate,
                   bool enabled = true,
                   bool visible = true);

    bool removeItem(ItemId id);
    void clear();

    // access items / look them up
    Item* getItem(ItemId id);
    const Item* getItem(ItemId id) const;
    const std::vector<Item>& items() const;

    // functions for dynamic options
    void setItemEnabled(ItemId id, bool enabled);
    void setItemVisible(ItemId id, bool visible);
    void setItemLabel(ItemId id, std::string label);
    //void setItemTooltip(ItemId id, std::string tooltip);
    void setEnabledPredicate(ItemId id, Predicate pred);
    void setVisiblePredicate(ItemId id, Predicate pred);
    void setActivateAction(ItemId id, Action action);
    void setHoverAction(ItemId id, Action action);
    void setSelectedAction(ItemId id, Action action);

    // select / navigate functions
    bool select(ItemId id);
    bool selectNext();
    bool selectPrevious();
    bool selectFirst();
    bool selectLast();
    void hover(ItemId id);

    // activate selections
    bool activateSelected();
    bool activate(ItemId id);

    // input handling
    InputResult handleNav(NavEvent event);

    // rendering items
    std::vector<RenderItem> buildRenderModel() const;

    // menu structure
    void setTitle(std::string title);
    const std::string& title() const;
    void setOpen(bool open);
    bool isOpen() const;
    void setIgnoreDisabledActivation(bool ignore);
    bool ignoreDisabledActivation() const;

private:
    // helpers
    bool isVisible(const Item& item) const;
    bool isEnabled(const Item& item) const;
    bool isSelectable(const Item& item) const;
    void normalizeSelection();
    ItemId nextId();

    // data members
    std::string m_title;
    bool m_open{true};
    bool m_ignoreDisabledActivation{true};
    std::vector<Item> m_items;
    std::unordered_map<std::string, ItemId> m_keyToId;
    ItemId m_nextId{1};
};

#endif