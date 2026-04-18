# Specification for ``agentlang``

## Types

- ``bool`` **:** Boolean value, ``true`` or ``false``, returnable
- ``int`` **:** Integer value, truthy if nonzero, returnable
- ``double`` **:** Floating-point value, truthy if nonzero, returnable
- ``str`` **:** String, truthy if nonempty, returnable
- ``point`` **:** Pair of integers, not bool-convertible, returnable
- ``direction`` **:** Enum of states ``left``, ``right``, ``up``, ``down``, not bool convertible, returnable
- ``car`` **:** Agent type in the traffic world, not bool convertible, not returnable
- ``student`` **:** Agent type in the infection world, not bool convertible, not returnable

## Statements

### Comment
A single-line comment is denoted by a double-slash, after which all characters on that line will be ignored:
```
// This is a comment
```

### World configuration
The first non-comment line in the script must define which of the two simulation environments the script is intended for. The world configuration follows the form ``world <infection/traffic>;``. 

Examples:
```
// "Infection" world is being simulated
world infection;
```
```
// "Traffic" world is being simulated
world traffic;
```

### Variable declaration
Variables may be declared but left undefined, but must be defined when referenced in simulation. A variable may be declared following the form ``let <variable name> : <variable type>;``.

Examples:
```
// Declare a variable representing the number of steps taken by the agent "bob"
let bobs_steps : int;

// Declare a variable representing the direction to be taken next
let next_direction : direction;
```

### Variable definition
Variables may be declared and defined at the same time following the form ``let <const/mut> <variable name> : <variable type> = <value/expression>;``. A variable marked ``const`` must always be given a value.

Examples:
```
// Declare a variable representing the number of steps taken by the agent "bob"
let bobs_steps : int = 0;

// Declare a variable representing the direction to be taken next
let next_direction : direction = LEFT;

// Declare a global constant variable representing max fractional occupancy
// that the agent "bob" finds acceptable
let const bob_max_occupancy : double = 0.5;
```

> Note: If `const` we'll have to require the value determinable at "interpretation" time?

### Agent declaration
Agents may be declared but left undefined, but must be defined later somewhere in the script. An agent declaration follows the same form as a variable declaration, where the ``<variable type>`` is ``car`` or ``student``. Agents may not be marked ``const``.

Examples:
```
// Declare bob to be a student
let bob : student;

// Declare equinox to be a car
let equinox : car;
```

### Agent definition
Agents are defined by a method ``init`` describing how the agent should be initialized, and a method ``turn`` describing how the agent should take its turn in simulation. An agent is defined following the form:
```
let bob : car {
	init : {
		// ...
	};
	turn : {
		// ...
	};
};
```

#### Agent spawn point
TODO

#### Agent world-reference
Agents may refer to information about their state in the world within the ``turn`` body following the form of a function invocation, passing in itself or another agent.

Examples:
```
// TODO
```

##### Valid world-references
###### Traffic
TODO
###### Infection
TODO

#### Agent self-reference
Agents may refer to variables defined in either ``init`` or ``turn`` by accessing ``self``, following the form ``self.<variable name>``.

Examples:
```
// TODO
```

#### Agent other-reference
Agents may refer to variables defined on other agents' ``init`` or ``turn`` by accessing the agents' names, following the form ``<agent name>.<variable name>``. Variables under other agents may only be referred to, not changed.

Examples:
```
// TODO
```

> Note: Might desire later to have the ability to change values under other agents

#### Agent actions
Agents may declare a move in the world by calling ``move()`` and passing in a ``direction``. 

Examples:
```
// An agent which only moves up
let move_upper : student {
	init : {
		// ...
	};
	turn : {
		move(UP);
	};
};
```
### Conditions
Same ``if/if-else/if-else if-else`` as ``C++``.

### Loops
Same ``while`` as ``C++``.
