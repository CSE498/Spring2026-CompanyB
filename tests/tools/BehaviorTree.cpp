#include "catch2/catch.hpp"
#include "tools/BehaviorTree/BehaviorTree.hpp"
#include <iostream>

// ATTRIBUTIONS: Used ChatGPT to create initial test cases. Further modifications come from my input

TEST_CASE("BehaviorTree construction and node insertion", "[tree][insert]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto node1 = std::make_unique<InvertNode>("Inv1");
    auto node2 = std::make_unique<InvertNode>("Inv2");

    REQUIRE(rootPtr->addNode(std::move(node1)));
    REQUIRE(rootPtr->addNode(std::move(node2)));

    // Duplicate move should fail
    REQUIRE_FALSE(rootPtr->addNode(std::move(node1)));
}

TEST_CASE("Decorator accepts single child and prevents duplicates", "[decorator]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv = std::make_unique<InvertNode>("Inv");
    InvertNode* invPtr = inv.get();

    REQUIRE(rootPtr->addNode(std::move(inv)));

    auto act1 = std::make_unique<ActionNode>("Act1");

    REQUIRE(invPtr->addNode(std::move(act1)));

    auto act2 = std::make_unique<ActionNode>("Act2");

    // Should fail because decorator already has a child
    REQUIRE_FALSE(invPtr->addNode(std::move(act2)));
}


TEST_CASE("Tick propagates correctly through tree (Sequence Fail)", "[tick]") {
    /*
    Tree:
        SelRoot
          Inv1
            Act1
          Inv2
            Act2
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(inv2));

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    inv1Ptr->addNode(std::move(act1));
    inv2Ptr->addNode(std::move(act2));

    REQUIRE(tree.tick() == 0);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);
    
    // Second branch will short circuit because Inv1's status is zero which makes SeqRoot fail
    REQUIRE(inv2Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);
}

TEST_CASE("Tick propagates correctly through tree (Sequence Pass)", "[tick]") {
    /*
    Tree:
        SeqRoot
          Act1
          Act2
          Act2
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    auto act3 = std::make_unique<ActionNode>("Act2");
    ActionNode* act3Ptr = act3.get();

    rootPtr->addNode(std::move(act1));
    rootPtr->addNode(std::move(act2));
    rootPtr->addNode(std::move(act3));

    REQUIRE(tree.tick() == 1);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(act2Ptr->tickCount() == 1);

    // Third branch ticks
    REQUIRE(act3Ptr->tickCount() == 1);
}

TEST_CASE("Tick propagates correctly through tree (Sequence Running)", "[tick]") {
    /*
    Tree:
        SeqRoot
          Act1
          Rep1
            Act2
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    RepeatNode* rep1Ptr = rep1.get();

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    rootPtr->addNode(std::move(act1));
    rootPtr->addNode(std::move(rep1));

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    rep1Ptr->addNode(std::move(act2));

    // Act1 has status of 1 and Rep1 has status of -1, but we dont know if Rep1 will be 0 or 1
    // So SeqRoot's status will be -1
    REQUIRE(tree.tick() == -1);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);
}

TEST_CASE("Tick propagates correctly through tree (Select Fail)", "[tick]") {
    /*
    Tree:
        SelRoot
          Inv1
            Act1
          Inv2
            Act2
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(inv2));

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    inv1Ptr->addNode(std::move(act1));
    inv2Ptr->addNode(std::move(act2));

    REQUIRE(tree.tick() == 0);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(inv2Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);
}

TEST_CASE("Tick propagates correctly through tree (Select Fail-2)", "[tick]") {
    /*
    Tree:
        SelRoot
          Rep1
            Inv1
              Act1
          Rep2
            Inv2
              Act2
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    auto rep2 = std::make_unique<RepeatNode>("Rep2");

    RepeatNode* rep1Ptr = rep1.get();
    RepeatNode* rep2Ptr = rep2.get();

    REQUIRE(rootPtr->addNode(std::move(rep1)));
    REQUIRE(rootPtr->addNode(std::move(rep2)));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    REQUIRE(rep1Ptr->addNode(std::move(inv1)));
    REQUIRE(rep2Ptr->addNode(std::move(inv2)));

    auto act1 = std::make_unique<ActionNode>("Act1");
    auto act2 = std::make_unique<ActionNode>("Act2");

    ActionNode* act1Ptr = act1.get();
    ActionNode* act2Ptr = act2.get();

    REQUIRE(inv1Ptr->addNode(std::move(act1)));
    REQUIRE(inv2Ptr->addNode(std::move(act2)));

    // Execute one tick
    REQUIRE(tree.tick() == 0);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(rep2Ptr->tickCount() == 1);
    REQUIRE(inv2Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);
}

TEST_CASE("Tick propagates correctly through tree (Select Pass)", "[tick]") {

    /*
    Tree:
        SelRoot
          Inv1
            Act1
          Act2
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    InvertNode* inv1Ptr = inv1.get();

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(act2));

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    inv1Ptr->addNode(std::move(act1));

    REQUIRE(tree.tick() == 1);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(act2Ptr->tickCount() == 1);
}

TEST_CASE("Tick propagates correctly through tree (Select Running)", "[tick]") {
    /*
    Tree:
        SelRoot
          Inv1
            Act1
          Rep1
            Act2
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    InvertNode* inv1Ptr = inv1.get();

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    RepeatNode* rep1Ptr = rep1.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(rep1));

    auto act1 = std::make_unique<ActionNode>("Act1");
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2");
    ActionNode* act2Ptr = act2.get();

    inv1Ptr->addNode(std::move(act1));
    rep1Ptr->addNode(std::move(act2));

    // Inv1 has status of 0 and Rep1 has status of -1, but we dont know if Rep1 will be 0 or 1
    // So SelRoot's status will be -1
    REQUIRE(tree.tick() == -1);

    // Root ticks
    REQUIRE(rootPtr->tickCount() == 1);

    // First branch ticks
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    // Second branch ticks
    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);
}

TEST_CASE("Node deletion works correctly", "[delete]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(inv2));

    REQUIRE(rootPtr->deleteNode(inv2Ptr));
    REQUIRE(rootPtr->deleteNode(inv1Ptr));
}