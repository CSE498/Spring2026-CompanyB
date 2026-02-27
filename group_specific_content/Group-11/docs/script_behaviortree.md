# ``BehaviorTree``

<!--
- Brief introduction (developer's name, name of implemented class)
- General overview of implemented class, brief example use case(s) (just how it might be useful)
- Example usage (we can show a code snippet and talk through it here)
- Overview of failure modes (any use of expecteds or exceptions, any notes on error handling suggestions)
- Known limitations, if any (if you made any restrictions on how it's used, mention here) (e.g., I limit the permissible types in ActionMap to ones given at instantiation)
- Depending on time left, overview of more challenging internal implementation concepts (things you wrote that are maybe more complex but are a backbone to the class)
-->

## Introduction

Hey y'all, Lalit here, and today I will show you my implementation of the BehaviorTree class

## Broad overview
Behavior Trees helps map conditional logic for agents by using different nodes and excecuting actions in a tick based system

Nodes can be one of the following base types:

- ### Leaf Nodes: 
	- Nodes that perform an action within a given tick duration
	
- ### Parent Nodes:
	- **Decorator Nodes:** Wraps around a child and perform an operation on it
	- **Composite Nodes:** Holds multiple children and perform an operation on them
	- **The child in a parent can be any node**

Each nodes can pass (1), fail (0), or still be running (-1)



### Benefits of using a Behavior Tree:

- #### Extendibility (Add More Nodes Types):
	- Cycle (Composite): Continuously loops through children until it's status isn't running (-1)
	- Random (Composite): Randomly access children
	- Parallel (Composite): Runs all children at the same time
	- Limiter (Decorator): Limit how many times a child can execute

- #### Modularity:
	- Mix an match whatever nodes you want!
	- Easy to create complex behaviors from simple building blocks

I will now demonstrate whether an agent should pick up a Red Ball or a Blue Ball

### Usage example
```
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
```
In this demonstration, I create a tree with two branches each with an inverter node wrapped around an action. I used `tickCount()`, `getActivePath()`, and `debugView()` in my output to help visualize the tree's state as we advance through each tick by running `tick()` by entering `t`. 

After 2 ticks, we can see that the agent has decided not to pick the Red Ball, and after 2 more ticks, we can see that the agent has decided to not pick up the Blue Ball, and so the agent lives happily ever after without picking either ball. 

Once we reach a non running status on the root node, we can still advance the tick count and not have the state of the tree change.

You can run this example and more by going to the `Group-11/src/BehaviorTree` Folder in the `Group-11-BasicClassImplement` branch and running the provided Makefile with `make`
### Error handling

**Programmer Errors: (Asserts)**
- Trees cannot be empty
- Deleting nonexistent child in Composite Node
- Adding node to a Decorator which already has a child

**Runtime Errors: (Conditional Logic)**
- Accessing `nullptr` in Composite Nodes (UB)
### Limitations & restrictions
- Actions Nodes are just print statements and always pass once the tick durration hits zero
	- Actions can either pass or fail within a tick durration which will have to be handled accordingly 
- Memory Map has not been implemented
	- This will allow for the nodes to communicate with each other
	- I wanted to focus on the basic functionality such as creating a Behavior Tree and excecuting ticks with statuses being properly updated

