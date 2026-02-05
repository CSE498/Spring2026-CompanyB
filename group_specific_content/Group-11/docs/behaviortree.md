## Class: BehaviorTree (Lalit Chitibomma)

### 1) Class Description

`BehaviorTree` creates a tree of nodes where leaf nodes are either external states (to be read) or actions (to perform and return running/success/failure).

Nodes that are actions have several types such as composite nodes like “sequence” (run children in order until one fails) and “select” (run children until one succeeds) or decorator nodes such as “invert” or “continually repeat”

Load/Create from JSON files containing a Behavior Tree

### 2) Similar Standard Library Classes

- **`std::unique_ptr`**: Used for exclusive ownership and automatic memory management for the tree node.
- **`std::vector`**: Used to store child nodes for composite behaviors.
- **`std::function`**: Used to encapsulate action and condition callbacks in leaf nodes.
- **`std::stack`/`std::queue`**: Used for tree traversal and debugging.
- **`std::map`**: Used to implement the shared memory blackboard.
- **`std::print`**: Used for formatted debugging and introspection output

### 3) Key Functions

We plan to implement the following:

**Core Modification**
- **`addNode()`**: Adds a new node to a specified parent
- **`deleteNode()`**: Removes a new node from a specified parent along with it's subtrees
- **`loadTree(const std::string& path)`**: Loads and constructs a behavior tree from a JSON file, validates its schema, and initializes all node relationships.

**Accessors & Views**
- **`void getActivePath()`**: Prints the currently active path
- **`int tickCount()`**: Returns the number of times the specified node has been evaluated (ticked) during execution.
- **`void debugView(std::ostream& out) const`**: Outputs a formatted representation of the tree structure, execution state, and recent transitions to the provided output stream.


**Calling**
- **`int tick() virtual = 0`**: Runs the action and returns one of three statuses: Success (1), Failure (0), or Running (-1)

### 4) Error Conditions

#### (1) Programmer Error — Assert

- Adding children to leaf nodes
- Null or invalid node references
- Invalid node type usage
- Violations of tree structure constraints

#### (2) Recoverable Error — Exceptions / Optional
- Failure to load or parse JSON files
- File I/O errors
- Resource allocation failures
- Memory exhaustion during tree construction

#### (3) User-Level / Soft Errors — Return Condition
- Invalid tree schemas
- Unsupported node definitions
- Missing required configuration fields
- Incorrect file formats

### 5) Expected Challenges
- Debugging long-running or asynchronous behaviors
- Ensuring consistent behavior across complex tree structures
- Validating and parsing JSON configuration files
- Maintaining performance with large trees

### 6) Coordination with Other Groups
- **Group 20 (Data Analytics)**: Will need to interact with `ActionLog` by providing them detailed records of executed actions in the Behavior Tree using the JSON file format