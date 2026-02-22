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

