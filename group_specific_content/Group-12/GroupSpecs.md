# Group 11: Scripted Agents

**Class:** AnnotationSet (Nick McComb)

---

## 1) Class Description

An annotation set is a collection of tags or other string-based annotations associated with an object. Its goals are to provide a simple way of tracking arbitrary data, allowing AI agents to maintain a memory about other agents, items, or places that they have encountered.

---

## 2) Similar Standard Library Classes

The following standard library classes should be familiar with or used to inform the functionality of development:

- **`std::unordered_set<std::string>`**  
  Ideal for AnnotationSet due to quick insertion, removal, and lookup, while also ensuring uniqueness of tags automatically.

- **`std::unordered_map<Key, Values>`**  
  Provides the foundation for how TagManager may associate tags with multiple object identifiers.

- **`std::string`**  
  The data type used in tag representation.

---

## 3) Key Functions

Key functions to be implemented are:

- `void AddTag(const std::string& tag);`
- `void RemoveTag(const std::string& tag);`
- `bool HasTag(const std::string& tag) const;`
- `const std::unordered_set<std::string>& GetTags() const;`

---

## 4) Error Conditions

Error conditions to consider when implementing include:

- When removing a tag from AnnotationSet, does the tag even exist.  
  **Programmer error**

- If the tag doesn’t exist, what is returned?  
  **Programmer error**

- Returning AnnotationSet as read only  
  **Programmer error**

- Handling attempting to add duplicate tags  
  **Programmer error / user error**

- Handling empty or invalid tag  
  **Programmer error / user error**

---

## 5) Expected Challenges

I think integrating the AnnotationSet class with the TagManager class may be difficult if Landon and I don’t communicate and plan appropriately. Determining setup and structure before implementation will be key for us here.

---

## 6) Coordination with Other Groups

List of other groups’ C++ classes to coordinate with:

- TagManager
- ActionMap
- DataFileManager
- Assert
