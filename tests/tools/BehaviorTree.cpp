// #include "catch2/catch.hpp"

#include "tools/BehaviorTree/BehaviorTree.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

// ATTRIBUTIONS: Used ChatGPT to create initial test cases. Further
// modifications come from my input

using NodeFactory = cse498::NodeFactory;
using NodeType = cse498::NodeType;

// Helper to unwrap std::expected cleanly in tests
auto unwrap = [](auto&& exp) {
  REQUIRE(exp.has_value());
  return std::move(exp.value());
};

TEST_CASE("BehaviorTree Construction and Node Insertion", "[tree][insert]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));

  auto node1 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv1"));
  auto node2 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv2"));

  rootPtr->addNode(std::move(node1));
  rootPtr->addNode(std::move(node2));
  rootPtr->addNode(std::move(node1));
}

TEST_CASE("Inserting and Retrieving Entries in Blackboard (Basic)", "[blackboard]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  blackboard.setValue("One", 1);
  blackboard.setValue("Two", 2);

  auto result = blackboard.getValue("One");

  REQUIRE(result.has_value());
  REQUIRE(std::get<int>(result.value()) == 1);

  result = blackboard.getValue("Two");

  REQUIRE(result.has_value());
  REQUIRE(std::get<int>(result.value()) == 2);
}

TEST_CASE("Retrieving Missing Entries in Blackboard", "[blackboard]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  blackboard.setValue("One", 1.0);
  blackboard.setValue("Two", 2.0);

  auto result = blackboard.getValue("One");

  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 1.0);

  result = blackboard.getValue("Two");

  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 2.0);

  result = blackboard.getValue("Three");

  REQUIRE(result.error() == "Value not found");
}

TEST_CASE("Accessing Empty Children in Sequence Node", "[sequence]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  // tick #1
  REQUIRE(tree.getActivePath() == "SeqRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(rootPtr->tickCount() == 1);

  // tick #2
  REQUIRE(tree.getActivePath() == "SeqRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Children in Select Node", "[select]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  // tick #1
  REQUIRE(tree.getActivePath() == "SelRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(rootPtr->tickCount() == 1);

  // tick #2
  REQUIRE(tree.getActivePath() == "SelRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Child in Repeat Node", "[repeat]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Repeat, "RepRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  // tick #1
  REQUIRE(tree.getActivePath() == "RepRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(rootPtr->tickCount() == 1);

  // tick #2
  REQUIRE(tree.getActivePath() == "RepRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Accessing Empty Child in Invert Node", "[invert]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Invert, "InvRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  // tick #1
  REQUIRE(tree.getActivePath() == "InvRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(rootPtr->tickCount() == 1);

  // tick #2
  REQUIRE(tree.getActivePath() == "InvRoot");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(rootPtr->tickCount() == 2);
}

TEST_CASE("Decorator accepts single child and prevents duplicates",
          "[decorator]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto inv = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv"));
  Node* invPtr = inv.get();

  rootPtr->addNode(std::move(inv));

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);

  invPtr->addNode(std::move(act1));

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);

  // Should fail because decorator already has a child
  invPtr->addNode(std::move(act2));
}

TEST_CASE("Tick propagates correctly through tree (Basic Action)",
          "[tree][action][tick]") {
  /*
  Tree:
      SelRoot
        Act1
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  rootPtr->addNode(std::move(act1));

  // tick #1
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(act1Ptr->tickCount() == 1);

  // tick #2 (Done)
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(act1Ptr->tickCount() == 2);

  // tick #3
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(act1Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Even Action)",
          "[tree][action][tick]") {
  /*
  Tree:
      SelRoot
        Act1
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  blackboard.setValue("Init Val", 2);

  Action squareGenerator = []([[maybe_unused]] Blackboard& blackboard){ 
    auto result = blackboard.getValue("Init Val");
    if (result) {
      int val = std::get<int>(result.value());
      blackboard.setValue("Init Val", val * val);
      return Status::Success;
    }
    else{
      return Status::Failure;
    }
  };

  auto act1 = NodeFactory::createActionNode("Act1", squareGenerator, 2);
  Node* act1Ptr = act1.get();

  rootPtr->addNode(std::move(act1));

  // tick #1
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(act1Ptr->tickCount() == 1);

  auto result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<int>(result.value()) == 4);

  // tick #2 (Done)
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(act1Ptr->tickCount() == 2);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<int>(result.value()) == 16);

  // tick #3
  REQUIRE(tree.getActivePath() == "SelRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(act1Ptr->tickCount() == 3);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<int>(result.value()) == 256);
}

TEST_CASE("Tick propagates correctly through tree (123-Sum Action)",
          "[tree][action][tick]") {
  /*
  Tree:
      SelRoot
        Act1
        Act2
        Act3
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  blackboard.setValue("Init Val", 0.0);

  Action sumOne = []([[maybe_unused]] Blackboard& blackboard){ 
    auto result = blackboard.getValue("Init Val");
    if (result) {
      double val = std::get<double>(result.value());
      blackboard.setValue("Init Val", val + 1.0);
      return Status::Success;
    }
    else{
      return Status::Failure;
    }
  };

  Action sumTwo = []([[maybe_unused]] Blackboard& blackboard){ 
    auto result = blackboard.getValue("Init Val");
    if (result) {
      double val = std::get<double>(result.value());
      blackboard.setValue("Init Val", val + 2.0);
      return Status::Success;
    }
    else{
      return Status::Failure;
    }
  };

  Action sumThree = []([[maybe_unused]] Blackboard& blackboard){ 
    auto result = blackboard.getValue("Init Val");
    if (result) {
      double val = std::get<double>(result.value());
      blackboard.setValue("Init Val", val + 3.0);
      return Status::Success;
    }
    else{
      return Status::Failure;
    }
  };


  auto act1 = NodeFactory::createActionNode("Act1", sumOne, 2);
  Node* act1Ptr = act1.get();

  rootPtr->addNode(std::move(act1));

  auto act2 = NodeFactory::createActionNode("Act2", sumTwo, 2);
  Node* act2Ptr = act2.get();

  rootPtr->addNode(std::move(act2));

  auto act3 = NodeFactory::createActionNode("Act3", sumThree, 2);
  Node* act3Ptr = act3.get();

  rootPtr->addNode(std::move(act3));

  // tick #1
  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(act1Ptr->tickCount() == 1);
  REQUIRE(act2Ptr->tickCount() == 0);
  REQUIRE(act3Ptr->tickCount() == 0);


  auto result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 1.0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 0);
  REQUIRE(act3Ptr->tickCount() == 0);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 2.0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 1);
  REQUIRE(act3Ptr->tickCount() == 0);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 4.0);

  // tick #4
  REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);
  REQUIRE(act3Ptr->tickCount() == 0);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 6.0);

  // tick #5
  REQUIRE(tree.getActivePath() == "SeqRoot - Act3");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);
  REQUIRE(act3Ptr->tickCount() == 1);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 9.0);

  // tick #6 (Done)
  REQUIRE(tree.getActivePath() == "SeqRoot - Act3");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 6);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);
  REQUIRE(act3Ptr->tickCount() == 2);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 12.0);

  // tick #7
  REQUIRE(tree.getActivePath() == "SeqRoot - Act3");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 7);

  REQUIRE(act1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);
  REQUIRE(act3Ptr->tickCount() == 3);

  result = blackboard.getValue("Init Val");
  REQUIRE(result.has_value());
  REQUIRE(std::get<double>(result.value()) == 15.0);
}

TEST_CASE("Tick propagates correctly through tree (Simple Sequence)",
          "[tree][sequence][tick]") {
  /*
  Tree:
      SeqRoot
        Act1
        Act2
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);
  Node* act2Ptr = act2.get();

  rootPtr->addNode(std::move(act1));
  rootPtr->addNode(std::move(act2));

  // tick #1
  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(act1Ptr->tickCount() == 1);

  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 1);

  // tick #4 (Done)
  REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 2);

  // tick #5
  REQUIRE(tree.getActivePath() == "SeqRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Simple Select)",
          "[tree][select][tick]") {
  /*
  Tree:
      SelRoot
        Inv1
          Act1
        Act2
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto inv1 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv1"));
  Node* inv1Ptr = inv1.get();

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);
  Node* act2Ptr = act2.get();

  rootPtr->addNode(std::move(inv1));
  rootPtr->addNode(std::move(act2));

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  inv1Ptr->addNode(std::move(act1));

  // tick #1

  REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(inv1Ptr->tickCount() == 1);
  REQUIRE(act1Ptr->tickCount() == 1);

  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SelRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 1);

  // tick #4 (Done)
  REQUIRE(tree.getActivePath() == "SelRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 2);

  // tick #5
  REQUIRE(tree.getActivePath() == "SelRoot - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Success);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Sequence Running)",
          "[tree][sequence][tick]") {
  /*
  Tree:
      SeqRoot
        Act1
        Rep1
          Act2
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto rep1 = unwrap(NodeFactory::createParentNode(NodeType::Repeat, "Rep1"));
  Node* rep1Ptr = rep1.get();

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  rootPtr->addNode(std::move(act1));
  rootPtr->addNode(std::move(rep1));

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);
  Node* act2Ptr = act2.get();

  rep1Ptr->addNode(std::move(act2));

  // tick #1

  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(act1Ptr->tickCount() == 1);

  REQUIRE(rep1Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SeqRoot - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 1);
  REQUIRE(act2Ptr->tickCount() == 1);

  // tick #4
  REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);

  // tick #5
  REQUIRE(tree.getActivePath() == "SeqRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 3);
  REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Select Running)",
          "[tree][select][tick]") {
  /*
  Tree:
      SelRoot
        Inv1
          Act1
        Rep1
          Act2
  */

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto inv1 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv1"));
  Node* inv1Ptr = inv1.get();

  auto rep1 = unwrap(NodeFactory::createParentNode(NodeType::Repeat, "Rep1"));
  Node* rep1Ptr = rep1.get();

  rootPtr->addNode(std::move(inv1));
  rootPtr->addNode(std::move(rep1));

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);
  Node* act2Ptr = act2.get();

  inv1Ptr->addNode(std::move(act1));
  rep1Ptr->addNode(std::move(act2));

  // tick #1
  REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(inv1Ptr->tickCount() == 1);
  REQUIRE(act1Ptr->tickCount() == 1);

  REQUIRE(rep1Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SelRoot - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 1);
  REQUIRE(act2Ptr->tickCount() == 1);

  // tick #4
  REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);

  // tick #5
  REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 3);
  REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Tick propagates correctly through tree (Select Fail)",
          "[tree][select][tick]") {
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

  auto root = unwrap(NodeFactory::createParentNode(NodeType::Select, "SelRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto rep1 = unwrap(NodeFactory::createParentNode(NodeType::Repeat, "Rep1"));
  auto rep2 = unwrap(NodeFactory::createParentNode(NodeType::Repeat, "Rep2"));

  Node* rep1Ptr = rep1.get();
  Node* rep2Ptr = rep2.get();

  rootPtr->addNode(std::move(rep1));
  rootPtr->addNode(std::move(rep2));

  auto inv1 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv1"));
  auto inv2 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv2"));

  Node* inv1Ptr = inv1.get();
  Node* inv2Ptr = inv2.get();

  rep1Ptr->addNode(std::move(inv1));
  rep2Ptr->addNode(std::move(inv2));

  Action successAction = []([[maybe_unused]] Blackboard& blackboard){ return Status::Success; };

  auto act1 = NodeFactory::createActionNode("Act1", successAction, 2);
  Node* act1Ptr = act1.get();

  auto act2 = NodeFactory::createActionNode("Act2", successAction, 2);
  Node* act2Ptr = act2.get();

  inv1Ptr->addNode(std::move(act1));
  inv2Ptr->addNode(std::move(act2));

  // tick #1
  REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 1);

  REQUIRE(rep1Ptr->tickCount() == 1);
  REQUIRE(inv1Ptr->tickCount() == 1);
  REQUIRE(act1Ptr->tickCount() == 1);

  REQUIRE(rep2Ptr->tickCount() == 0);
  REQUIRE(inv2Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #2
  REQUIRE(tree.getActivePath() == "SelRoot - Rep1 - Inv1 - Act1");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 2);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep2Ptr->tickCount() == 0);
  REQUIRE(inv2Ptr->tickCount() == 0);
  REQUIRE(act2Ptr->tickCount() == 0);

  // tick #3
  REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Running);
  REQUIRE(tree.tickCount() == 3);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep2Ptr->tickCount() == 1);
  REQUIRE(inv2Ptr->tickCount() == 1);
  REQUIRE(act2Ptr->tickCount() == 1);

  // tick #4 (Done)
  REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Failure);
  REQUIRE(tree.tickCount() == 4);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep2Ptr->tickCount() == 2);
  REQUIRE(inv2Ptr->tickCount() == 2);
  REQUIRE(act2Ptr->tickCount() == 2);

  // tick #5
  REQUIRE(tree.getActivePath() == "SelRoot - Rep2 - Inv2 - Act2");
  REQUIRE(tree.tick(blackboard) == Status::Failure);
  REQUIRE(tree.tickCount() == 5);

  REQUIRE(rep1Ptr->tickCount() == 2);
  REQUIRE(inv1Ptr->tickCount() == 2);
  REQUIRE(act1Ptr->tickCount() == 2);

  REQUIRE(rep2Ptr->tickCount() == 3);
  REQUIRE(inv2Ptr->tickCount() == 3);
  REQUIRE(act2Ptr->tickCount() == 3);
}

TEST_CASE("Node deletion works correctly", "[delete]") {
  auto root = unwrap(NodeFactory::createParentNode(NodeType::Sequence, "SeqRoot"));
  Node* rootPtr = root.get();

  cse498::BehaviorTree tree(std::move(root));
  auto blackboard = tree.getBlackboard();

  auto inv1 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv1"));
  auto inv2 = unwrap(NodeFactory::createParentNode(NodeType::Invert, "Inv2"));

  Node* inv1Ptr = inv1.get();
  Node* inv2Ptr = inv2.get();

  rootPtr->addNode(std::move(inv1));
  rootPtr->addNode(std::move(inv2));

  rootPtr->deleteNode(inv2Ptr);
  rootPtr->deleteNode(inv1Ptr);

  REQUIRE(tree.getActivePath() == "SeqRoot");
}

