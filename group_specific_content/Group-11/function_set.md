## Class: Function Set (Daniel Petkoski)

### 1) Class Description
`FunctionSet` is a collection of functions (or other invocable objects such as member function, lamdas, functors, etc.) with the same signature. It will be able to be called with a set of arguments, and those arguments will be passed to the internal collection functions and called one at a time in the order they were added.

It's goals are to streamline the process of calling multiple functions with the same signaure, possibly making it easier and less verbose to call multiple functions in things like tight event loops.

### 2) Similar Standard Library Classes
* **`std::function`**: An `std::function` is used intermall in the `FunctionSet` to hold any function that matches it's signature. This is vital to those that require this functionality. Being familiar with `std::function` will make the class easier to use, as the API will be similar.

* **`std::vector`**: This is the underlying container that the functions will be stored in. We need it to be growable to support adding more functions dynamically.

### 3) Key Functions
We plan to implement the following:

#### Core Modification
* `void Add(T&& funcObj);` — Adds a function to the set.
* `void Clear();` — Resets everything.
* `bool Empty() const;` — Check if the set is empty.
* `size_t Size() const;` — Retreive the size of the set.

#### Accessors & Views
* `const T& At(size_t index) const;` — Safe access to a function.

#### Calling
* `void operator(Args&&... args) or void call(Args&&... args)` — call the functions in order.
* Perhaps other operations such as call_reverse, or maybe a method to add custom operations with lambdas.

### 4) Error Conditions

#### (1) Programmer Error — Assert
* Calling `Front()` or `Back()` on an empty set.

#### (2) Recoverable Error — Exceptions / Optional
* Accessing an index that doesn't exist can throw a runtime exception.
* **Calling** Calling an empty set will do nothing.

#### (3) User-Level / Soft Errors — Return Condition
* Incorrect template parameters will be caught at compile time
* Adding a function with the wrong signature will be caught at compile time
* Calling with the wrong arguments will be caught at compile time

### 5) Expected Challenges
* **Templates:** Writing correct and efficient template code with many types, variadic arguments, and more is something that I need to brush up on.
* **Ensuring temporary lambdas are stored correcly** Temporary lamdas get destroying if not copied correctly. I need to ensure my generic code handles this case.
* **Creating a good API** Creating the API to be flexible and useful is something that will require understanding the usecases for the `FunctionSet` further.

### 6) Coordination with Other Groups
* The `FunctionSet` should be generic enough to not need cooperation with any other group, since simply making an object callable satsifies the main requirement.
* Co-operating with **Group 13 (Math World)** and **Group 19 (Interaction Simulation World)** to see what they need for stuff like their event loop could help me make a more flexible and useful API for those teams in particular.
