/**
 * This file is the Menu class: a model that manages items, selection,
 * navigation, predicates (dynamic visibility/enabled/selections), and
 * callback actions for the GUI
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <expected>

namespace cse498 {

/**
 * Stores items identified by a programmer-facing string key and a stable ItemId.
 * Supports navigation events, predicate-driven dynamic visibility/enabled state,
 * and callback hooks.
 */
class Menu {
public:
    /**
     * Numeric identifier for menu items
     */
    using ItemId = std::uint32_t;

    /**
     * Callback type used for item actions
     * (activate, hover, selected)
     */
    using Action = std::function<void()>;

    /**
     * Predicate type for dynamic runtime state
     */
    using Predicate = std::function<bool()>;

    /**
     * Navigation events that menu can handle
     */
    enum class NavEvent {
        Up,
        Down,
        Left,
        Right,
        Activate,
        Back,
        Home,
        End
    };

    /**
     * Result of handling navigation event
     */
    enum class InputResult {
        Ignored,
        Heard
    };

    /**
     * Renderable snapshot of item's UI state
     */
    struct RenderItem {
        ItemId id{};
        std::string label{};
        bool enabled{true};
        bool visible{true};
        bool selected{false};
    };

    /**
     * Default constructor.
     */
    Menu();

    /**
     * Constructs a menu with a title.
     * @param title Menu title displayed by the GUI.
     */
    explicit Menu(std::string title);

    /**
     * Adds a new item or updates an existing item with the same key.
     *
     * @param key unique programmer facing identifier
     * @param label user facing label to display in the GUI
     * @param onActivate callback invoked when the item is activated
     * @param enabled base enabled flag
     * @param visible base visible flag
     * @return stable ItemId for inserted/updated item
     */
    ItemId addItem(const std::string& key,
                   const std::string& label,
                   Action onActivate,
                   bool enabled = true,
                   bool visible = true);

    /**
     * Removes an item by ItemId.
     */
    bool removeItem(ItemId id);

    /**
     * Removes an item by its string key.
     */
    bool removeItem(std::string_view key);

    /**
     * Removes all items and clears the key->id mapping.
     */
    void clear();

    /**
     * Finds an ItemId by item key.
     */
    std::optional<ItemId> findItemIdByKey(std::string_view key) const;

    /** Set an item's base enabled flag. */
    void setItemEnabled(ItemId id, bool enabled);

    /** Set an item's base visible flag. */
    void setItemVisible(ItemId id, bool visible);

    /** Set an item's label text. */
    void setItemLabel(ItemId id, std::string label);

    /** Set an item's enabled predicate. */
    void setEnabledPredicate(ItemId id, Predicate pred);

    /** Set an item's visible predicate. */
    void setVisiblePredicate(ItemId id, Predicate pred);

    /** Set an item's activation callback. */
    void setActivateAction(ItemId id, Action action);

    /** Set an item's hover callback. */
    void setHoverAction(ItemId id, Action action);

    /** Set an item's selection callback. */
    void setSelectedAction(ItemId id, Action action);

    /**
     * Select an item by ItemId.
     */
    std::expected<void, std::string> select(ItemId id);

    /**
     * Select the next selectable item.
     */
    bool selectNext();

    /**
     * Select the previous selectable item.
     */
    bool selectPrevious();

    /**
     * Select the first selectable item.
     */
    bool selectFirst();

    /**
     * Select the last selectable item.
     */
    bool selectLast();

    /**
     * Hover an item by ItemId.
     */
    bool hover(ItemId id);

    /**
     * Activate the currently selected item.
     */
    bool activateSelected();

    /**
     * Activate an item by ItemId.
     */
    std::expected<void, std::string> activate(ItemId id);

    /**
     * Handle a navigation event.
     */
    InputResult handleNav(NavEvent event);

    /**
     * Build a render snapshot for the GUI.
     */
    std::vector<RenderItem> buildRenderModel() const;

    /** Set the menu title. */
    void setTitle(std::string title);

    /** Get the menu title. */
    const std::string& title() const;

    /** Set whether the menu is open. */
    void setOpen(bool open);

    /** Get whether the menu is open. */
    bool isOpen() const;

    /** Set whether disabled items are blocked from activation. */
    void setIgnoreDisabledActivation(bool ignore);

    /** Get whether disabled items are blocked from activation. */
    bool ignoreDisabledActivation() const;

private:
    struct Item {
        ItemId id{};
        std::string key;
        std::string label;

        bool enabled{true};
        bool visible{true};
        bool selected{false};

        Predicate enabledIf{};
        Predicate visibleIf{};

        Action onActivate{};
        Action onHover{};
        Action onSelected{};
    };

    /**
     * First valid ItemId value
     */
    static constexpr ItemId kFirstId = 1;

    Item* getItem(ItemId id);
    const Item* getItem(ItemId id) const;
    Item* getItemByKey(std::string_view key);
    const Item* getItemByKey(std::string_view key) const;

    /**
     * Compute final visibility.
     */
    bool isVisible(const Item& item) const;

    /**
     * Compute final enabled state.
     */
    bool isEnabled(const Item& item) const;

    /**
     * Compute whether an item can be selected.
     */
    bool isSelectable(const Item& item) const;

    /**
     * Repair selection to ensure a valid selectable item is selected.
     */
    void normalizeSelection();

    /**
     * Generate next unique ItemId.
     */
    ItemId nextId();

    /**
     * Menu title shown by the GUI.
     */
    std::string m_title{""};

    /**
     * Whether the menu is currently open and accepts navigation input.
     */
    bool m_open{true};

    /**
     * If true, disabled items can't be activated.
     */
    bool m_ignoreDisabledActivation{true};

    /**
     * Ordered storage of menu items.
     */
    std::vector<Item> m_items;

    /**
     * Lookup from item key to ItemId.
     */
    std::unordered_map<std::string, ItemId> m_keyToId;

    /**
     * Id generator - never decreases or reuses Ids.
     */
    ItemId m_nextId{kFirstId};
};

}