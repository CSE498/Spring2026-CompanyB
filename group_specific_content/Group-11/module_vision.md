# Group 11: Scripted Agents

Our goal for the scripted agents module is to separate “what an agent can do” from “what an agent decides to do.” Instead of hard-coding if-else chains for every possible behavior, we are building a system where behaviors are assembled dynamically using a Behavior Tree and an Action Map. This means a user can change states without needing us to recompile the engine. By keeping movement logic in WorldPath separate from the decision logic (BehaviorTree), we can build complex and intelligent agents that are easy to debug and flexible enough to work with in any of the company’s different simulations (whether its a traffic simulator or a biological ecosystem).

