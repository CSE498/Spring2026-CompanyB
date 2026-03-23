
## Class: ActionMap (Cole Scheller)

### Class Description 
``ActionMap`` offers a map matching a string to a callable function, allowing the programmer to dynamically select and invoke arbitrary functions at runtime. The functionality of this class largely mimics the idea of a [Dispatch Table](https://en.wikipedia.org/wiki/Dispatch_table).

### Similar STD classes
- ``std::unordered_map`` (and its ordered counterpart) provide a generic map implementation
- ``std::function`` provides a templated representation of a callable

### Error Conditions
> Note: Recoverable errors represented in code as states under an enum ``ActionMapErr``.
- Too few arguments / ``TOO_FEW_ARGS`` : Likely user error, may be recoverable. 
- Too many arguments / ``TOO_MANY_ARGS`` : Likely user error, may be recoverable.
- No matching callable / ``CALLABLE_NOT_FOUND`` : Likely user error, may be recoverable
- Invoked function error / ``INVOCATION_ERR`` : Possibly recoverable
- Invalid argument type(s) / ``INVALID_ARG_TYPE`` : Likely user error, may be recoverable
- Invalid return type / ``INVALID_RET_TYPE`` : User error, recoverable
- Callable name already exists / ``NAME_EXISTS`` : User error, recoverable

### Key Functions
#### Registration
- Register a callable : ``register_callable(std::string name, std::function<{return type}({args...})>)``
  - Returns``std::expected<std::string, ActionMapErr>``supplying the name if successful
- De-register a callable : ``deregister_callable(std::string name)  ``
  - Returns``std::expected<std::string, ActionMapErr>``supplying the name if successful
  
#### Map State
- Check a name is registered : ``exists(std::string name) -> bool``
- Determine how many callables are currently registered : ``size() -> size_t``

#### Invocation
- Invoke a registered callable : ``invoke<Ret>(std::string, {args...})``
  - Returns ``std::expected<Ret, ActionMapErr>``
  
#### Compile-Time
- Check if type ``T`` is permissible within a defined ``ActionMap`` : ``is_valid_type<T>() -> constexpr bool``
- Check if function signature ``RetType(...ArgTypes)`` is permissible within a defined ``ActionMap`` : ``is_valid_signature<Ret(...ArgTypes)> -> constexpr bool``
  
### Expected Challenges
Primary challenge up front is largely around how to keep the allowable functions flexible -- how to keep the map agnostic to function signatures. One solution may be to create a templated "box" object to wrap around function signatures, and allow the box methods to handle invocation. This is just one early solution, further research into implementations of dispatch tables in C++ are likely to shine some light on alternative methods. 

### Other Group's C++ Classes
- Group 20's ``Menu``, as it associates GUI options with a function to be called. On its surface, it would appear that ``Menu`` would benefit from ``ActionMap``'s implementation.
