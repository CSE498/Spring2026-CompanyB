## ``<CLASS_NAME>``

<!--
- Brief introduction (developer's name, name of implemented class)
- General overview of implemented class, brief example use case(s) (just how it might be useful)
- Example usage (we can show a code snippet and talk through it here)
- Overview of failure modes (any use of expecteds or exceptions, any notes on error handling suggestions)
- Known limitations, if any (if you made any restrictions on how it's used, mention here) (e.g., I limit the permissible types in ActionMap to ones given at instantiation)
- Depending on time left, overview of more challenging internal implementation concepts (things you wrote that are maybe more complex but are a backbone to the class)
-->

### Introduction

- Introduce self
- Introduce implemented class

### Broad overview
- Give broad description of implemented class
  - [ Fill in bullets for broad description ]
  - [ What is it? What is its utility? What pain point does it solve? ]
- Give example use cases
  - [ Fill in bullets for possible use cases ]
  - [ Can be broad, just want to describe how it might be useful ]
### Usage example
- Give a stub of a meaningful basic usage of your class
  - [ Fill in code block with basic usage ]
  - [ Fill in bullets explaining the usage example ]
	- [ Describe the broad strokes of your example, note any nuances like instantiation or whatever ]
### Error handling
- Give description of how you handle errors, and how you expect users to handle them
  - [ Fill in bullets for errors which may originate from your class ]
	- [ (basically just give the main ways someone can misuse your class) ]
  - [ Fill in bullets for how you handle errors ]
  - [ Do you use ``std::expected``? What should the user do with the error results? ]
  - [ Do you use exceptions? When should the user expect to handle thrown exceptions? ]
### Limitations & restrictions
- Give description any known limitations in your design, or restrictions you made for it
  - [ Fill in bullets for known limitations ]
  - [ Fill in bullets for restrictions made ]
	- [ If restrictions were made, fill in bullets for why ]
	- [ (essentially what tradeoffs did you make and why) ]

