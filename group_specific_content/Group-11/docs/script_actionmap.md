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

