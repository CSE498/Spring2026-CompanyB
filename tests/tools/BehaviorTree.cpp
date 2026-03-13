#include "catch2/catch.hpp"
#include "tools/BehaviorTree/BehaviorTree.hpp"
#include <iostream>

// ATTRIBUTIONS: Used ChatGPT to create initial test cases. Further modifications come from my input

TEST_CASE("BehaviorTree Construction and Node Insertion", "[tree][insert]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto node1 = std::make_unique<InvertNode>("Inv1");
    auto node2 = std::make_unique<InvertNode>("Inv2");

    rootPtr->addNode(std::move(node1));
    rootPtr->addNode(std::move(node2));
    rootPtr->addNode(std::move(node1));
}

TEST_CASE("Accessing Empty Children in Sequence Node", "[sequence]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    // tick #1
    REQUIRE(tree.getActivePath() == "SeqRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);

    REQUIRE(rootPtr->tickCount() == 1);

    // tick #2
    REQUIRE(tree.getActivePath() == "SeqRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Children in Select Node", "[select]") {

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    // tick #1
    REQUIRE(tree.getActivePath() == "SelRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);
    
    REQUIRE(rootPtr->tickCount() == 1);

    // tick #2
    REQUIRE(tree.getActivePath() == "SelRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Child in Repeat Node", "[repeat]") {

    auto root = std::make_unique<RepeatNode>("RepRoot");
    RepeatNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    // tick #1
    REQUIRE(tree.getActivePath() == "RepRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);

    REQUIRE(rootPtr->tickCount() == 1);

    // tick #2
    REQUIRE(tree.getActivePath() == "RepRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Child in Invert Node", "[invert]") {

    auto root = std::make_unique<InvertNode>("InvRoot");
    InvertNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    // tick #1
    REQUIRE(tree.getActivePath() == "InvRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);

    REQUIRE(rootPtr->tickCount() == 1);

    // tick #2
    REQUIRE(tree.getActivePath() == "InvRoot");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Decorator accepts single child and prevents duplicates", "[decorator]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto inv = std::make_unique<InvertNode>("Inv");
    InvertNode* invPtr = inv.get();

    rootPtr->addNode(std::move(inv));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);

    invPtr->addNode(std::move(act1));

    auto act2 = std::make_unique<ActionNode>("Act2", 2);

    // Should fail because decorator already has a child
    invPtr->addNode(std::move(act2));
}

TEST_CASE("Tick propagates correctly through tree (Action)", "[tree][action][tick]") {
    /*
    Tree:
        SelRoot
          Act1
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    ActionNode* act1Ptr = act1.get();

    rootPtr->addNode(std::move(act1));

    // tick #1
    REQUIRE(tree.getActivePath() == "SelRoot - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);

    REQUIRE(act1Ptr->tickCount() == 1);

    // tick #2 (Done)
    REQUIRE(tree.getActivePath() == "SelRoot - Act1");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(act1Ptr->tickCount() == 2);

    // tick #3
    REQUIRE(tree.getActivePath() == "SelRoot - Act1");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(act1Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Simple Sequence)", "[tree][sequence][tick]") {
    /*
    Tree:
        SeqRoot
          Act1
          Act2
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2", 2);
    ActionNode* act2Ptr = act2.get();

    rootPtr->addNode(std::move(act1));
    rootPtr->addNode(std::move(act2));

    // tick #1
    REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);
    
    REQUIRE(act1Ptr->tickCount() == 1);

    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #2
    REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #3
    REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 1);

    // tick #4 (Done)
    REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 4);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 2);

    // tick #5
    REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 5);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Simple Select)", "[tree][select][tick]") {
    /*
    Tree:
        SeqRoot
          Inv1
            Act1
          Act2
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    InvertNode* inv1Ptr = inv1.get();

    auto act2 = std::make_unique<ActionNode>("Act2", 2);
    ActionNode* act2Ptr = act2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(act2));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    ActionNode* act1Ptr = act1.get();

    inv1Ptr->addNode(std::move(act1));

    // tick #1

    REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);
    
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #2
    REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #3
    REQUIRE(tree.getActivePath() == "SelRoot - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(act2Ptr->tickCount() == 1);

    // tick #4 (Done)
    REQUIRE(tree.getActivePath() == "SelRoot - Act2");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 4);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);
    
    REQUIRE(act2Ptr->tickCount() == 2);

    // tick #5
    REQUIRE(tree.getActivePath() == "SelRoot - Act2");
    REQUIRE(tree.tick() == Status::Success);
    REQUIRE(tree.tickCount() == 5);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);
    
    REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Sequence Running)", "[tree][sequence][tick]") {
    /*
    Tree:
        SeqRoot
          Act1
          Rep1
            Act2
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    RepeatNode* rep1Ptr = rep1.get();

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    ActionNode* act1Ptr = act1.get();

    rootPtr->addNode(std::move(act1));
    rootPtr->addNode(std::move(rep1));

    auto act2 = std::make_unique<ActionNode>("Act2", 2);
    ActionNode* act2Ptr = act2.get();

    rep1Ptr->addNode(std::move(act2));

    // tick #1

    REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);
    
    REQUIRE(act1Ptr->tickCount() == 1);

    REQUIRE(rep1Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #2
    REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #3
    REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);

    // tick #4
    REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 4);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(act2Ptr->tickCount() == 2);

    // tick #5
    REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 5);

    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 3);
    REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Select Running)", "[tree][select][tick]") {
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

    cse498::BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    InvertNode* inv1Ptr = inv1.get();

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    RepeatNode* rep1Ptr = rep1.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(rep1));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    ActionNode* act1Ptr = act1.get();

    auto act2 = std::make_unique<ActionNode>("Act2", 2);
    ActionNode* act2Ptr = act2.get();

    inv1Ptr->addNode(std::move(act1));
    rep1Ptr->addNode(std::move(act2));

    // tick #1
    REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);
    
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    REQUIRE(rep1Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #2
    REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #3
    REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);

    // tick #4
    REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 4);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(act2Ptr->tickCount() == 2);

    // tick #5
    REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 5);

    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 3);
    REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Select Fail)", "[tree][select][tick]") {
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

    cse498::BehaviorTree tree(std::move(root));

    auto rep1 = std::make_unique<RepeatNode>("Rep1");
    auto rep2 = std::make_unique<RepeatNode>("Rep2");

    RepeatNode* rep1Ptr = rep1.get();
    RepeatNode* rep2Ptr = rep2.get();

    rootPtr->addNode(std::move(rep1));
    rootPtr->addNode(std::move(rep2));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    rep1Ptr->addNode(std::move(inv1));
    rep2Ptr->addNode(std::move(inv2));

    auto act1 = std::make_unique<ActionNode>("Act1", 2);
    auto act2 = std::make_unique<ActionNode>("Act2", 2);

    ActionNode* act1Ptr = act1.get();
    ActionNode* act2Ptr = act2.get();

    inv1Ptr->addNode(std::move(act1));
    inv2Ptr->addNode(std::move(act2));

    // tick #1
    REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 1);

    REQUIRE(rep1Ptr->tickCount() == 1);
    REQUIRE(inv1Ptr->tickCount() == 1);
    REQUIRE(act1Ptr->tickCount() == 1);

    REQUIRE(rep2Ptr->tickCount() == 0);
    REQUIRE(inv2Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #2
    REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Inv1 - Act1");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 2);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep2Ptr->tickCount() == 0);
    REQUIRE(inv2Ptr->tickCount() == 0);
    REQUIRE(act2Ptr->tickCount() == 0);

    // tick #3
    REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
    REQUIRE(tree.tick() == Status::Running);
    REQUIRE(tree.tickCount() == 3);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep2Ptr->tickCount() == 1);
    REQUIRE(inv2Ptr->tickCount() == 1);
    REQUIRE(act2Ptr->tickCount() == 1);

    // tick #4 (Done)
    REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
    REQUIRE(tree.tick() == Status::Failure);
    REQUIRE(tree.tickCount() == 4);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep2Ptr->tickCount() == 2);
    REQUIRE(inv2Ptr->tickCount() == 2);
    REQUIRE(act2Ptr->tickCount() == 2);

    // tick #5
    REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
    REQUIRE(tree.tick() == Status::Failure);
    REQUIRE(tree.tickCount() == 5);

    REQUIRE(rep1Ptr->tickCount() == 2);
    REQUIRE(inv1Ptr->tickCount() == 2);
    REQUIRE(act1Ptr->tickCount() == 2);

    REQUIRE(rep2Ptr->tickCount() == 3);
    REQUIRE(inv2Ptr->tickCount() == 3);
    REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Node deletion works correctly", "[delete]") {

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    auto inv2 = std::make_unique<InvertNode>("Inv2");

    InvertNode* inv1Ptr = inv1.get();
    InvertNode* inv2Ptr = inv2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(inv2));

    rootPtr->deleteNode(inv2Ptr);
    rootPtr->deleteNode(inv1Ptr);

    REQUIRE(tree.getActivePath() == "SeqRoot");
}