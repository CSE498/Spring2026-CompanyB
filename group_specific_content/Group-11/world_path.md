
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
- `void AddPoint(const Point& p);`  
  **Input:** A valid `Point` (finite coordinates).
  **Behavior:** Appends point to the path.  
  **Assert:** Coordinates must not be NaN/Inf.

- `void Clear();`  
  **Behavior:** Removes all stored points.

- `bool Empty() const;`  
  **Returns:** `true` if no points exist.

- `void PopBack();`  
  **Behavior:** Removes last point.  
  **Assert:** Path must not be empty.

- `void Reserve(size_t n);`  
  **Behavior:** Pre-allocates memory for performance when generating paths.

#### Accessors & Views
- `size_t Size() const;`  
  **Returns:** Number of stored points.

- `const Point& At(size_t index) const;`  
  **Input:** Valid index.  
  **Returns:** Reference to that point.  
  **Throws:** `std::out_of_range` if invalid.

- `const Point& Front() const;`  
  **Assert:** Path not empty.  
  **Returns:** First point.

- `const Point& Back() const;`  
  **Assert:** Path not empty.  
  **Returns:** Last point.

- `auto GetSegments() const;`  
  **Returns:** A `std::views::adjacent<2>` view of consecutive point pairs.

---

#### Geometry / Metrics
- `double TotalLength() const;`  
  **Returns:** Sum of all segment distances.  
  **Edge Case:** Returns `0.0` if fewer than 2 points.

- `std::optional<double> SegmentLength(size_t index) const;`  
  **Returns:** Length of segment `index → index+1`.  
  **Returns:** `std::nullopt` if invalid.

- `bool SelfIntersects() const;`  
  **Returns:** Whether any non-adjacent segments intersect.  
  (Full implementation may be added later.)

- `bool IsValid() const;`  
  **Returns:** True if all stored points are finite.

---

#### Serialization / Debug
- `void LogPath() const;`  
  **Behavior:** Prints all stored points using `std::print`.
---

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
