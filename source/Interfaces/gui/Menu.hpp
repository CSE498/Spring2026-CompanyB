// Menu header file
// Group 21 GUI Interface

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cse498 {

class Menu {
public:
    using ItemId = std::uint32_t;

    // callbacks owned by menu items
    using Action = std::function<void()>;

    // used for dynamic state rules (ex. visible)
    using Predicate = std::function<bool()>;

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

    enum class InputResult {
        Ignored, // menu closed or event not used
        Heard    // menu handled it
    };

    // gui version of item state
    struct RenderItem {
        ItemId id{};
        std::string label{};
        bool enabled{true};
        bool visible{true};
        bool selected{false};
    };

    // storage for menu items
    struct Item {
        ItemId id{};
        std::string key;    // programmer sie
        std::string label;  // user facing text

        bool enabled{true};
        bool visible{true};
        bool selected{false};

        Predicate enabledIf{};
        Predicate visibleIf{};

        Action onActivate{};
        Action onHover{};
        Action onSelected{};
    };

    Menu();
    explicit Menu(std::string title);

    // add or update by key
    ItemId addItem(std::string key,
                   std::string label,
                   Action onActivate,
                   bool enabled = true,
                   bool visible = true);

    bool removeItem(ItemId id);
    void clear();

    // const overload for const methods like buildrendermodel
    Item* getItem(ItemId id);
    const Item* getItem(ItemId id) const;
    const std::vector<Item>& items() const;

    void setItemEnabled(ItemId id, bool enabled);
    void setItemVisible(ItemId id, bool visible);
    void setItemLabel(ItemId id, std::string label);
    void setEnabledPredicate(ItemId id, Predicate pred);
    void setVisiblePredicate(ItemId id, Predicate pred);
    void setActivateAction(ItemId id, Action action);
    void setHoverAction(ItemId id, Action action);
    void setSelectedAction(ItemId id, Action action);

    bool select(ItemId id);
    bool selectNext();
    bool selectPrevious();
    bool selectFirst();
    bool selectLast();
    void hover(ItemId id);

    bool activateSelected();
    bool activate(ItemId id);

    InputResult handleNav(NavEvent event);

    // gui calls this
    std::vector<RenderItem> buildRenderModel() const;

    void setTitle(std::string title);
    const std::string& title() const;
    void setOpen(bool open);
    bool isOpen() const;
    void setIgnoreDisabledActivation(bool ignore);
    bool ignoreDisabledActivation() const;

private:
    static constexpr ItemId kFirstId = 1;

    bool isVisible(const Item& item) const;
    bool isEnabled(const Item& item) const;
    bool isSelectable(const Item& item) const;
    void normalizeSelection();
    ItemId nextId();

    std::string m_title;
    bool m_open{true};
    bool m_ignoreDisabledActivation{true};
    std::vector<Item> m_items;
    std::unordered_map<std::string, ItemId> m_keyToId;
    ItemId m_nextId{kFirstId};
};

} 


