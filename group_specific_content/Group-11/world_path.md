
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
