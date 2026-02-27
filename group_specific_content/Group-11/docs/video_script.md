# Group 11 Video Script

Link to video: https://mediaspace.msu.edu/media/CSE+498+Company+B+Group+11+Video/1_o9iz7iet

## `WorldPath`

### Introduction

Hey everyone, I'm Devansh. I worked on `WorldPath`: it's a class that keeps track of where an agent has been or where it's headed.

### Broad overview

It's an ordered list of 2D points that bundles in the geometric math you'd want — total path length, interpolating a position along the path, checking for self-intersection. `PathGenerator` builds one of these, hands it to an Agent, and the Agent queries it directly.

I used C++23 ranges: `std::views::pairwise` gives consecutive point pairs without index loops, and `fold_left` sums distances.

### Usage example

```C++
// quick demo for the screen recording
// g++ -std=c++23 -I src demo_worldpath.cpp -o demo_worldpath

#include "src/WorldPath.hpp"
#include <iomanip>
#include <iostream>

int main() {
  std::cout << std::fixed << std::setprecision(1);

  cse498::WorldPath path;
  path.addPoint({0.0, 0.0});
  path.addPoint({3.0, 0.0});
  path.addPoint({3.0, 4.0});

  std::cout << "totalLength: " << path.totalLength() << "\n";

  auto pt = path.pointAtDistance(5.0);
  std::cout << "pointAtDistance 5: (" << pt.x() << ", " << pt.y() << ")\n";

  auto seg = path.segmentLength(0);
  if (seg)
    std::cout << "segmentLength(0): " << *seg << "\n";
  else
    std::cout << "segmentLength(0): nullopt\n";

  std::cout << "isClosed:" << std::boolalpha << path.isClosed() << "\n";

  return 0;
}
```

I'm making an L-shaped path — three points. `totalLength` gives 7, that's 3 across plus 4 up. `pointAtDistance` is the key one for agents: you say "I've traveled 5 units" and it interpolates exactly where that puts you. `segmentLength` returns an optional, so an out-of-range index gives you nullopt instead of a crash.

### Error handling

Two categories. Programmer bugs — calling `front()` on an empty path, adding NaN coordinates — those assert. Shouldn't happen if you're using it right.

Runtime edge cases return something useful: missing segment gives an empty optional, `totalLength` on an empty path gives zero, and `at` throws if the index is bad.

### Limitations

`selfIntersects` and `furthestPair` are O(n²). Fine for our path sizes. And `addPoint` asserts on NaN so you catch bad data early.

## ``PathGenerator``

### Introduction

Hey everyone, I'm Chirag. I worked on `PathGenerator`: it's the class that computes routes for agents and hands them back as `WorldPath` objects.

### Broad overview

The idea is to centralize all navigation logic in one place so agents don't have to figure out their own routes — they just ask for a path and follow it. `PathGenerator` supports several path types: A\* shortest paths, patrol loops through waypoints, avoidance paths that route around a region, random walks for wandering behavior, and expanding spirals for search patterns.

The world interface is kept abstract through a `WorldQueryFunc` callback — a simple `bool(const Point&)` that answers "can I move here?" That means `PathGenerator` doesn't care whether you're on a grid, a continuous field, or a physics simulation. You plug in whatever predicate makes sense for your world.

### Usage example

```cpp
#include "src/PathGenerator.hpp"
#include <iostream>

int main() {
  cse498::PathGenerator gen;

  // Open world — everything is walkable
  auto openWorld = [](const cse498::Point &) { return true; };

  // A* shortest path
  auto path = gen.ShortestPath({0, 0}, {5, 5}, openWorld);
  if (path) {
    std::cout << "Shortest path points: " << path->size() << "\n";
    std::cout << "Total length: " << path->totalLength() << "\n";
  }

  // Patrol loop through three waypoints
  auto patrol = gen.PatrolPath({{0,0}, {4,0}, {4,4}}, /*loop=*/true);
  if (patrol)
    std::cout << "Patrol path points: " << patrol->size() << "\n";

  // Avoidance — route from (0,0) to (6,0), stay 2 units away from (3,0)
  auto avoid = gen.AvoidancePath({0,0}, {6,0}, {3,0}, 2.0, openWorld);
  if (avoid)
    std::cout << "Avoidance path found, length: " << avoid->totalLength() << "\n";
  else
    std::cout << "No avoidance path found\n";

  return 0;
}
```

`ShortestPath` runs A\* and returns an `std::optional<WorldPath>` — if no route exists you get `nullopt`, otherwise a ready-to-use path. `PatrolPath` stitches waypoints together with A\* segments and, with `loop=true`, closes the circuit back to the start. `AvoidancePath` wraps the same A\* but with an extra predicate that rejects any point inside the avoidance radius — so you just describe the zone to stay out of and the algorithm figures out the detour.

### Error handling

Two tiers. Programmer bugs — null `WorldQueryFunc`, negative avoidance radius, non-positive step size — those assert. You shouldn't hit them in normal use.

Runtime edge cases return something useful: if no path exists between start and goal (blocked world, avoidance zone cuts off all routes, search exceeds the 10000 iteration cap), you get `std::nullopt`. `RandomWalk` never fails — if it hits a dead end it just returns whatever partial path it managed to build. `PatrolPath` silently skips unreachable waypoints and connects what it can.

### Limitations & restrictions

A\* here is 8-directional and grid-stepped — diagonal moves cost the same as cardinal, which is fine for agents but isn't true continuous pathfinding. The 10000 iteration cap prevents runaway searches on large maps but means very long paths may return `nullopt` even when a route technically exists; lower the step size only when you need fine-grained paths and the world is small. `SpiralPath` and `RandomWalk` ignore the `canMove` callback for the start point validation in the spiral case — those are purely geometric generators. Path smoothing is not implemented, so returned paths will follow the step grid rather than cutting corners.

## ``ActionMap``

### Introduction

- Introduce self
  - Cole Scheller
- Introduce implemented class
  - ActionMap

### Broad overview
- Give broad description of implemented class
  - `ActionMap` offers a map matching a string to a callable function, allowing the programmer to dynamically select and invoke arbitrary functions at runtime
  - The functionality of this class largely mimics the idea of a dispatch table
- Give example use cases
  - A web interface dropdown of actions
### Usage example
- Give a stub of a meaningful basic usage of your class
```cpp

#include <functional>
#include <string>

// Return whether the string s as a double is equal to d
bool str_dbl_cmp(std::string s, double d) {
	return (std::stod(s) == d);
}

int main() {
	using namespace cse498;
	ActionMap<std::string, int, double> demo_map{};
	
	// Insert named function
	// auto here will be std::expected<str, ActionMapErr>
	auto reg_res = demo_map.register_callable("compare_double", str_dbl_cmp);
	if (!res.has_value()) {
		// Handle error
	}
	
	auto invoke_res = demo_map.invoke<bool>("compare_double", "1.0", 1.0);
	if (!res.has_value()) {
		// Handle error
	} else {
		std::cout << "Result: " << invoke_res.value() << std::endl;
	}
	
	
	// Register func to check double equality
	// Let EPS be some very small range
	reg_res = demo_map.register_callable("double_eq",
		[] (double a, double b) {
			double diff = a - b;
			return (-EPS < diff && diff < EPS);
		}
	);
	
	auto second_invoke_res = demo_map<bool>("double_eq", 1.0, 2.0);
	if (!res.has_value()) {
		// Handle error
	} else {
		std::cout << "Result: " << second_invoke_res << std::endl;
	}
}

```
  <!-- - [ Fill in code block with basic usage ] -->
  <!-- - [ Fill in bullets explaining the usage example ] -->
  <!-- 	- [ Describe the broad strokes of your example, note any nuances like instantiation or whatever ] -->
### Error handling
- Give description of how you handle errors, and how you expect users to handle them
  - No exceptions
  - ``std::expected`` with an enum of errors in ``ActionMapErr``
  - These can be:
	- Invalid signature on invocation (TooFew, TooMany, InvalidRet, InvalidArg)
	- Callable not found / Name already exists
	- Other undetermined error at invocation
   - Some errors handled via compilation error
	 - Only types defined for a given ActionMap are permissible, trying to pass other types in will fail to compile
 - Users can choose to handle errors as they see fit, as fatal errors are handled via compile error
### Limitations & restrictions
- Give description any known limitations in your design, or restrictions you made for it
  - As mentioned prior, a given actionmap is restricted to permit only the types specified when instantiated
  - This restriction allows for runtime type-checking through the use of a variant, which eliminates the need for costly exceptions


## ``BehaviorTree``

<!--
- Brief introduction (developer's name, name of implemented class)
- General overview of implemented class, brief example use case(s) (just how it might be useful)
- Example usage (we can show a code snippet and talk through it here)
- Overview of failure modes (any use of expecteds or exceptions, any notes on error handling suggestions)
- Known limitations, if any (if you made any restrictions on how it's used, mention here) (e.g., I limit the permissible types in ActionMap to ones given at instantiation)
- Depending on time left, overview of more challenging internal implementation concepts (things you wrote that are maybe more complex but are a backbone to the class)
-->

### Introduction

Hey y'all, Lalit here, and today I will show you my implementation of the BehaviorTree class

### Broad overview
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


## ``FunctionSet``

<!--
- Brief introduction (developer's name, name of implemented class)
- General overview of implemented class, brief example use case(s) (just how it might be useful)
- Example usage (we can show a code snippet and talk through it here)
- Overview of failure modes (any use of expecteds or exceptions, any notes on error handling suggestions)
- Known limitations, if any (if you made any restrictions on how it's used, mention here) (e.g., I limit the permissible types in ActionMap to ones given at instantiation)
- Depending on time left, overview of more challenging internal implementation concepts (things you wrote that are maybe more complex but are a backbone to the class)
-->

### Introduction

- Introduce self (Hello I'm Daniel Petkoski, etc.)
- Introduce implemented class (I wrote the FunctionSet class).

### Broad overview
- Give broad description of implemented class
  - The FunctionSet is a container of functions that all have the same signature.
  - The FunctionSet allows for each of its stored functions to be called one at a time.
  - It is iterable and indexable as well, allowing for the access of functions in different ways.
  - Its use is useful if you have many different functions that need to called when something happens (i.e action, event loop).
- Give example use cases
  - First, cover the template parameters (Ret, Params...) 
  - Demonstrate adding functions
  - Note how all of the functions are different (Free, Lambda, Member)
  - Demonstrate invoking functions
  - Show the order the functions are called
  - Demonstrate iterating, things like size(), clear()

### Usage example
- Give a stub of a meaningful basic usage of your class
  - Show function set with different functions taking a reference and mutating it
  - Show final result
### Error handling
- Give description of how you handle errors, and how you expect users to handle them
  - Be careful passing temporaries, as they may be consumed by a function and then become invalid for the next function
  - Indexing with operator[] is UB
  - Normal invoke() propogates exceptions
  - invoke\_all returns an std::expected if any function throws detailing the function that threw
### Limitations & restrictions
- Give description any known limitations in your design, or restrictions you made for it
  - No return type aggregation becuase its hard to deduce what types can be stored in a vector to return
  - Decided to not allow stuff like resize() since this is a high-level container, storing and doing stuff efficiently is less important since stuff will not be added as often.
