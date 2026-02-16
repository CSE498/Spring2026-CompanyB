#include "../../sources/tools/BehaviorTree/BehaviorTree.hpp"

#include <cassert>
#include <iostream>

// int main() {
//     // Create BehaviorTree
//     std::unique_ptr<SequenceNode> root = std::make_unique<SequenceNode>("SeqRoot");
//     SequenceNode* rootPtr = root.get();

//     BehaviorTree tree = BehaviorTree(std::move(root));

//     // Add Nodes
//     std::unique_ptr<InvertNode> node1 = std::make_unique<InvertNode>("Inv1");
//     InvertNode* node1Ptr = node1.get();

//     std::unique_ptr<InvertNode> node2 = std::make_unique<InvertNode>("Inv2");
//     InvertNode* node2Ptr = node2.get();

//     assert(  rootPtr->addNode(std::move(node1)) );
//     assert(  rootPtr->addNode(std::move(node2)) );
//     assert( !rootPtr->addNode(std::move(node1)) ); // No Duplicate Nodes

//     std::unique_ptr<ActionNode> node3 = std::make_unique<ActionNode>("Act1");
//     ActionNode* node3Ptr = node3.get();

//     assert( node1Ptr->addNode(std::move(node3)) );

//     std::unique_ptr<ActionNode> node4 = std::make_unique<ActionNode>("Act2");
//     ActionNode* node4Ptr = node4.get();

//     assert(  node2Ptr->addNode(std::move(node4)) );
//     assert( !node1Ptr->addNode(std::move(node4)) ); // No Duplicate Nodes

//     // View Behavior Tree
//     tree.debugView();
//     tree.tick();

//     // Node Tick Count
//     std::cout << rootPtr->tickCount() << '\n';
//     std::cout << node1Ptr->tickCount() << '\n';
//     std::cout << node2Ptr->tickCount() << '\n';
//     std::cout << node3Ptr->tickCount() << '\n';
//     std::cout << node4Ptr->tickCount() << '\n';


//     // Delete Nodes
//     assert( node2Ptr->deleteNode(node3Ptr) );
//     assert( node1Ptr->deleteNode(node4Ptr) );

//     assert(  rootPtr->deleteNode(node2Ptr) );
//     assert(  rootPtr->deleteNode(node1Ptr) );

//     return 0;
// }

int main() {
    // Create BehaviorTree
    std::unique_ptr<SequenceNode> root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree = BehaviorTree(std::move(root));

    std::unique_ptr<ActionNode> node1 = std::make_unique<ActionNode>("Act1");
    ActionNode* node1Ptr = node1.get();

    assert( rootPtr->addNode(std::move(node1)) );

    std::unique_ptr<ActionNode> node2 = std::make_unique<ActionNode>("Act2");
    ActionNode* node2Ptr = node2.get();

    assert(  rootPtr->addNode(std::move(node2)) );
    assert( !rootPtr->addNode(std::move(node1)) ); // No Duplicate Nodes

    // View Behavior Tree
    tree.debugView();
    tree.tick();

    // Node Tick Count
    std::cout << rootPtr->tickCount() << '\n';
    std::cout << node1Ptr->tickCount() << '\n';
    std::cout << node2Ptr->tickCount() << '\n';

    // Delete Nodes
    assert(  rootPtr->deleteNode(node2Ptr) );
    assert(  rootPtr->deleteNode(node1Ptr) );

    return 0;
}