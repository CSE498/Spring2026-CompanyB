# Group 11: Scripted Agents

Our goal for the scripted agents module is to separate “what an agent can do” from “what an agent decides to do.” Instead of hard-coding if-else chains for every possible behavior, we are building a system where behaviors are assembled dynamically using a Behavior Tree and an Action Map. This means a user can change states without needing us to recompile the engine. By keeping movement logic in WorldPath separate from the decision logic (BehaviorTree), we can build complex and intelligent agents that are easy to debug and flexible enough to work with in any of the company’s different simulations (whether its a traffic simulator or a biological ecosystem).

## Class: WorldPath (Devansh Tayal)

### 1) Class Description
`WorldPath` keeps track of where an agent has been or where it's headed. Instead of just passing around a raw `std::vector<Point>`, we're making a class that holds the path data and does the math.

The main goal is to use **C++23 Ranges** for geometric calculations (like path length or intersections) without copying data or writing ugly loops. This is what gets passed from `PathGenerator` (which builds the route) to the Agent (which follows it). We're keeping the interface clean so other groups can query the path—like checking bounds or getting the next point—without needing to know how the points are stored internally.

### 2) Similar Standard Library Classes
* **`std::vector<Point>`**: This is what we'll use to actually store the points. We want contiguous memory for cache reasons.
* **`std::views::adjacent` (C++23)**: We're using this to make path math easier. Instead of writing loops like `for(i=0; i<size-1; ++i)` to get line segments, `adjacent` gives us pairs of points directly. It's safer and cleaner.
* **`std::optional` (C++17)**: We'll use this for functions that might fail (like asking for a segment length that doesn't exist) instead of throwing exceptions or returning magic numbers like `-1`.
* **`std::print` (C++23)**: We're replacing `std::cout` with `std::print` because formatting is way easier.

### 3) Key Functions
We plan to implement the following:

#### Core Modification
* `void AddPoint(const Point& p);` — Adds a point to the path.
* `void Clear();` — Resets everything.
* `bool Empty() const;` — Check if path has points.

#### Accessors & Views
* `const Point& At(size_t index) const;` — Safe access to a point.
* `auto GetSegments() const;` — **(C++23)** Returns a `std::views::adjacent` view. This way, other classes (like Physics) can iterate over the "lines" of the path directly without us maintaining a separate list of edges.

#### Geometry / Metrics
* `double TotalLength() const;` — Sums the distance of all segments.
* `std::optional<double> SegmentLength(size_t index) const;` — Returns the length if the index is valid, or `std::nullopt` if not.
* `bool SelfIntersects() const;` — Checks if the path crosses itself.
* `BoundingBox Bounds() const;` — Returns the min/max X and Y.

#### Serialization
* `void LogPath() const;` — Uses **`std::print`** to dump the path to the console for debugging.

### 4) Error Conditions

#### (1) Programmer Error — Assert
* Accessing an index that doesn't exist.
* Calling `Front()` or `Back()` on an empty path.
* Passing `NaN` coordinates to `AddPoint`.

#### (2) Recoverable Error — Exceptions / Optional
* **File Loading:** If we implement `LoadFromFile` and it fails, we'll throw a `std::runtime_error` because the program shouldn't keep going if it can't load the file.
* **Invalid Queries:** `SegmentLength(i)` on the last point returns `std::nullopt`, so the caller can deal with it.

#### (3) User-Level / Soft Errors — Return Condition
* `TotalLength()` on an empty path just returns `0.0`.
* Asking for the "Next Point" when an agent is already at the end returns a status flag saying it's done.

### 5) Expected Challenges
* **Floating Point Precision:** Checking if two lines intersect is tricky with `double` because of precision errors. We'll need an epsilon check or we'll get false positives everywhere.
* **Performance:** Checking for self-intersections is slow (O(N²)). If paths get really long, we might need to optimize by only checking the most recent segments.
* **Compiler Support:** We're assuming the environment supports C++23. If `std::print` or `views::adjacent` aren't working in the company compiler, we'll have to fall back to C++20 formatting and regular loops.

### 6) Coordination with Other Groups
* **Group 13 (Math World):** We're using their `Point` struct. We need to make sure we agree on the coordinate system (Cartesian vs. Polar).
* **Group 11 (Internal):** `PathGenerator` creates these objects, so our functions need to match what they expect.
* **Group 20 (Data Analytics):** They need to replay our paths, so we need to give them a string format they can parse easily.

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
