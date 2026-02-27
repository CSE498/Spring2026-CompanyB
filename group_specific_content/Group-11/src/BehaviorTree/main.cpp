#include "tools/BehaviorTree/BehaviorTree.hpp"

#include <iostream>

cse498::BehaviorTree createBasicSequence() {
    /*
    Tree:
        SeqRoot
          Grab Ball
          Chuck It
    */

    auto root = std::make_unique<SequenceNode>("SeqRoot");
    SequenceNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto act1 = std::make_unique<ActionNode>("Grab Ball", 2);
    auto act2 = std::make_unique<ActionNode>("Chuck It", 2);

    rootPtr->addNode(std::move(act1));
    rootPtr->addNode(std::move(act2));

    return tree;
}

cse498::BehaviorTree createBasicSelect() {
    /*
    Tree:
        SeqRoot
          Inv1
            Grab Blue Ball
          Inv2
            Grab Red Ball
    */

    auto root = std::make_unique<SelectNode>("SelRoot");
    SelectNode* rootPtr = root.get();

    cse498::BehaviorTree tree(std::move(root));

    auto inv1 = std::make_unique<InvertNode>("Inv1");
    InvertNode* inv1Ptr = inv1.get();

    auto inv2 = std::make_unique<InvertNode>("Inv2");
    InvertNode* inv2Ptr = inv2.get();

    rootPtr->addNode(std::move(inv1));
    rootPtr->addNode(std::move(inv2));

    auto act1 = std::make_unique<ActionNode>("Grab Red Ball", 2);
    auto act2 = std::make_unique<ActionNode>("Grab Blue Ball", 2);

    inv1Ptr->addNode(std::move(act1));
    inv2Ptr->addNode(std::move(act2));

    return tree;
}

cse498::BehaviorTree createSelectFail() {
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

    inv1Ptr->addNode(std::move(act1));
    inv2Ptr->addNode(std::move(act2));

    return tree;
}

int main() {
    std::string line = "\n_______________________________________\n";

    auto tree = createBasicSelect();

    std::cout << "Initial Tree: \n";

    std::string active_path = "Current Path: " + tree.getActivePath();

    std::cout << line + "Tick #" << tree.tickCount() << ":" + line;
    std::cout << active_path << "\n\n";
    tree.debugView();
    std::cout << "\n";

    // See if there are any keys waiting in standard input (wait if needed)
    char input;

    // Quit program with 'q'
    while(input != 'q') {    
        do {
        std::cin >> input;
        } while (!std::cin);

        // Increment tick with 't'
        if (input == 't') {
            std::string active_path = "Current Path: " + tree.getActivePath();

            Status status = tree.tick();

            std::cout << line + "Tick #" << tree.tickCount() << ":" + line;
            std::cout << active_path << "\n\n";

            tree.debugView();

            if (tree.tickCount() < 4) std::cout << line;

            else if (tree.tickCount() >= 4 && tree.tickCount() < 10) {
                if (status == Status::Success or status == Status::Failure) { std::cout << "\nThat's it! Keep going ig..." + line; }
            }

            else if (tree.tickCount() >= 10 && tree.tickCount() < 15) std::cout << line + "\nway to go..." + line;

            else if (tree.tickCount() >= 15) std::cout << line + "\nwow..." + line;

        }
    }

    return 0;
}