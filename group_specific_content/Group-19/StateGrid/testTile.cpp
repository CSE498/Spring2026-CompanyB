// There has to ba irony in the testing file being ai generated
// but here we are

#include <iostream>
#include <cassert>

#include "Tile.hpp"

int main() {
    MetaData md;
    md.movementModifier = 2;
    md.condition = Condition::Wet;

    Tile t(0, 0, 'P', "Path", md);

    // Initially empty
    assert(t.getAgent(0, 0) == nullptr);

    auto a1 = std::make_shared<Agent>(1);

    // Place agent in (0,0) subcell
    bool ok = t.addAgent(a1, 0, 0);
    assert(ok);

    // Occupied cell should reject
    auto a2 = std::make_shared<Agent>(2);
    bool ok2 = t.addAgent(a2, 0, 0);
    assert(!ok2);

    // Out-of-bounds should reject
    bool ok3 = t.addAgent(a2, 3, 0);
    assert(!ok3);

    // Lookup should return the same underlying pointer
    const Agent* found = t.getAgent(0, 0);
    assert(found != nullptr);
    assert(found == a1.get());
    assert(found->getId() == 1);

    // Out-of-bounds lookup should be nullptr
    assert(t.getAgent(-1, 0) == nullptr);
    assert(t.getAgent(0, 3) == nullptr);

    std::cout << "All Tile tests passed!\n";
    std::cout << "Tile (" << t.getRow() << "," << t.getColumn() << ") "
              << "name=" << t.getName() << " symbol=" << t.getSymbol() << "\n";
    std::cout << "Meta: movementModifier=" << t.getMetaData().movementModifier << "\n";

    return 0;
}
