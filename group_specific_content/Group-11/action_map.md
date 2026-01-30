
## Class: ActionMap (Cole Scheller)

### Class Description 
``ActionMap`` offers a map matching a string to a callable function, allowing the programmer to dynamically select and invoke arbitrary functions at runtime. The functionality of this class largely mimics the idea of a [Dispatch Table](https://en.wikipedia.org/wiki/Dispatch_table).

### Similar STD classes
- ``std::unordered_map`` (and its ordered counterpart) provide a generic map implementation
- ``std::function`` provides a templated representation of a callable

### Error Conditions
> Note: Recoverable errors represented in code as states under an enum ``ActionMapErr``.
- Too few arguments / ``TooFewArgs`` : Likely user error, may be recoverable. 
- Too many arguments / ``TooManyArgs`` : Likely user error, may be recoverable.
- No matching callable / ``NoMatchingRegistered`` : Likely user error, may be recoverable
- Invoked function error / ``InvokedError`` : Possibly recoverable
- Invalid argument types / ``TypeError`` : Likely user error, may be recoverable

### Key Functions
#### Registration
- Register a callable ``Register(std::string name, std::function<{return type}({args})>)``
  - Returns``std::expected<std::string, ActionMapErr>``supplying the name if successful
- De-register a callable ``Deregister(std::string name)  ``
  - Returns``std::expected<std::string, ActionMapErr>``supplying the name if successful
- Check a name is registered ``Exists(std::string name) -> bool``

#### Invocation
- Invoke a registered callable ``Invoke(std::string, {args})``
  - Returns ``std::expected<{callable return}, ActionMapErr>``
  
### Expected Challenges
Primary challenge up front is largely around how to keep the allowable functions flexible -- how to keep the map agnostic to function signatures. One solution may be to create a templated "box" object to wrap around function signatures, and allow the box methods to handle invocation. This is just one early solution, further research into implementations of dispatch tables in C++ are likely to shine some light on alternative methods. 

### Other Group's C++ Classes
- Group 20's ``Menu``, as it associates GUI options with a function to be called. On its surface, it would appear that ``Menu`` would benefit from ``ActionMap``'s implementation.
