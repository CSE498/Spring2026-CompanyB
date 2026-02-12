
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
- `void addPoint(const Point& p);`  
  **Input:** A valid `Point` (finite coordinates).
  **Behavior:** Appends point to the path.  
  **Assert:** Coordinates must not be NaN/Inf.

- `void clear();`  
  **Behavior:** Removes all stored points.

- `bool empty() const;`  
  **Returns:** `true` if no points exist.

- `bool popBack();`  
  **Behavior:** Removes last point.  
  **Returns:** `false` if already empty.

- `void reserve(size_t n);`  
  **Behavior:** Pre-allocates memory for performance when generating paths.

#### Accessors & Views
- `size_t size() const;`  
  **Returns:** Number of stored points.

- `Point& operator[](size_t index);` / `const Point& operator[](size_t index) const;`  
  **Behavior:** Unchecked access (like `std::vector`).

- `Point& at(size_t index);` / `const Point& at(size_t index) const;`  
  **Returns:** Reference to that point.  
  **Throws:** `std::out_of_range` if invalid.

- `Point& front();` / `const Point& front() const;`  
  **Assert:** Path not empty.  
  **Returns:** First point.

- `Point& back();` / `const Point& back() const;`  
  **Assert:** Path not empty.  
  **Returns:** Last point.

- `std::span<const Point> pointsView() const;`  
  **Returns:** Zero-copy view of the stored points.

- `auto begin()/end()` (const + non-const)  
  **Behavior:** Standard iteration over points.

- `auto segments() const;`  
  **Returns:** A `std::views::pairwise` view of consecutive point pairs.

---

#### Geometry / Metrics
- `double totalLength() const;`  
  **Returns:** Sum of all segment distances.  
  **Edge Case:** Returns `0.0` if fewer than 2 points.

- `std::optional<double> segmentLength(size_t index) const;`  
  **Returns:** Length of segment `index → index+1`.  
  **Returns:** `std::nullopt` if invalid.

- `std::pair<Point, Point> furthestPair() const;`  
  **Returns:** The two points with maximum distance (O(N²)).  
  **Assert:** Path must contain at least 2 points.

- `bool isClosed(double eps = 1e-9) const;`  
  **Returns:** True if first and last points are the same within epsilon.

- `Point pointAtDistance(double target) const;`  
  **Returns:** The interpolated point at a distance along the path.  
  **Assert:** Path must not be empty.

- `bool selfIntersects() const;`  
  **Returns:** Whether any non-adjacent segments intersect (O(N²)).  
  **Note:** Closed paths (last == first) do not count the closing segment as self-intersection.

- `bool isValid() const;`  
  **Returns:** True if all stored points are finite.

#### Composition
- `void append(const WorldPath& other);`  
  **Behavior:** Appends another path to the end of this one.

- `WorldPath reversed() const;`  
  **Returns:** A new path with points in reverse order.

---

---

### 4) Error Conditions

#### (1) Programmer Error — Assert
* Accessing an index that doesn't exist.
* Calling `front()` or `back()` on an empty path.
* Passing `NaN` coordinates to `addPoint`.

#### (2) Recoverable Error — Exceptions / Optional
* **File Loading:** If we implement `LoadFromFile` and it fails, we'll throw a `std::runtime_error` because the program shouldn't keep going if it can't load the file.
* **Invalid Queries:** `segmentLength(i)` on the last point returns `std::nullopt`, so the caller can deal with it.

#### (3) User-Level / Soft Errors — Return Condition
* `totalLength()` on an empty path just returns `0.0`.
* Asking for the "Next Point" when an agent is already at the end returns a status flag saying it's done.

### 5) Expected Challenges
* **Floating Point Precision:** Checking if two lines intersect is tricky with `double` because of precision errors. We'll need an epsilon check or we'll get false positives everywhere.
* **Performance:** Checking for self-intersections is slow (O(N²)). If paths get really long, we might need to optimize by only checking the most recent segments.
* **Compiler Support:** We're assuming the environment supports C++23. If `std::print` or `views::adjacent` aren't working in the company compiler, we'll have to fall back to C++20 formatting and regular loops.

### 6) Coordination with Other Groups
* **Group 13 (Math World):** We're using their `Point` struct. We need to make sure we agree on the coordinate system (Cartesian vs. Polar).
* **Group 11 (Internal):** `PathGenerator` creates these objects, so our functions need to match what they expect.
* **Group 20 (Data Analytics):** They need to replay our paths, so we need to give them a string format they can parse easily.
