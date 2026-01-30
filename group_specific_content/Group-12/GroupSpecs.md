# Group 12: Scripted Agents

---

**Class:** AnnotationSet (Nick McComb)

## 1) Class Description

An annotation set is a collection of tags or other string-based annotations associated with an object. Its goals are to provide a simple way of tracking arbitrary data, allowing AI agents to maintain a memory about other agents, items, or places that they have encountered.


## 2) Similar Standard Library Classes

The following standard library classes should be familiar with or used to inform the functionality of development:

- **`std::unordered_set<std::string>`**  
  Ideal for AnnotationSet due to quick insertion, removal, and lookup, while also ensuring uniqueness of tags automatically.

- **`std::unordered_map<Key, Values>`**  
  Provides the foundation for how TagManager may associate tags with multiple object identifiers.

- **`std::string`**  
  The data type used in tag representation.


## 3) Key Functions

Key functions to be implemented are:

- `void AddTag(const std::string& tag);`
- `void RemoveTag(const std::string& tag);`
- `bool HasTag(const std::string& tag) const;`
- `const std::unordered_set<std::string>& GetTags() const;`


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


## 5) Expected Challenges

I think integrating the AnnotationSet class with the TagManager class may be difficult if Landon and I don’t communicate and plan appropriately. Determining setup and structure before implementation will be key for us here.


## 6) Coordination with Other Groups

List of other groups’ C++ classes to coordinate with:

- TagManager
- ActionMap
- DataFileManager
- Assert

---

**Class:** RobinHoodMap (John Stouffer)


## 1) Class Description

A Robinhood Map is a type of HashMap that is faster than the standard libraries **std::unordered_map**. The point of this class is so that if we every need a hashmap of some sort it will likely be more useful to use this data structure rather than **std::unordered_map**

## 2) Similar Standard Library Classes

The following standard library classes should be familiar with or used to inform the functionality of development:

- **`std::unordered_map<Key, Values>`**  

Although this is the data structure we are attempting to replace, this will be one of our primary references. 

- **`std::vector`**

A possible underlying data structure for the robinhood map. Referencing `std::vector` will likely be important for memory management, iterators, etc.

- **`std::string`**  
  
This is the most likely case for a key value. Other likely data types can and will be used but they will be more intuitive while the `std::string` class has a lot methods to know.

- **`std::hash`**

This is the standard hashing function that the standard template uses. Will likely need to know how this works for the keys

- **`std::tuple`** 

We will use std::tuple instead of std::pair for the key and value pairs. Because I have only heard bad things about std::pair. I will do more research before concretely putting this, but we shall see.

## 3) Key Functions

Key functions to be implemented are:

- `bool insert(const std::tuple<Key, Value>& kv);`
- `Value& operator[](const Key& key);`
- `const Value& at(const Key& key) const;`
- `iterator find(const Key& key);`
- `const_iterator find(const Key& key) const;`
- `bool erase(const Key& key);`
- `size_t size() const noexcept;`
- `void clear() noexcept;`
- `iterator begin();`
- `iterator end();`
- `bool empty() const noexcept;`


## 4) Error Conditions

Error conditions to consider when implementing RobinHoodMap include:

- Accessing a key with at() that doesn't exist in the map.
**Recoverable error**

- Dereferencing an invalid or end() iterator.
**Programmer error (assert)**

- Using an iterator after the map has been rehashed/modified.
**Programmer error (document undefined behavior)**

- Running out of memory during insertion or rehashing.
**Recoverable error**

- Searching for a key with find() that doesn't exist.
**User error (return end())**

- Attempting to erase() a key that doesn't exist.
**User error (return 0)**

- Calling operator[] on a Value type that isn't default constructible.
**Programmer error (static_assert at compile time)**

## 5) Expected Challenges

I think the biggest challenge I will run into will be for one the complexity of the algorithm. Strategies for resizing, rehashing. Iterators will esepcially be a challenge given the iterator invalidation.

## 6) Coordination with Other Groups

List of other groups’ C++ classes to possibly would need to coordinate with:

- DataFileManager
- Assert
- StateGridPosition


---


**Class:** TagManager (Landon Cosby)

## 1) Class Description

An indexing service that tracks relationships between tags and objects, tags being string type labels, objects identified using a unique ID. Its main goal is to make tag-based lookups fast and easy, including multi-tag searches. 

## 2) Similar Standard Library Classes

-`std::unordered_map`
Will store the tag as the key and the value as the unique ID of the objects with the tags.
-`std::unordered_set`
Set of unique IDs that will be used with unordered map
-`std::vector`
Used to return unique ID’s for several functions


## 3) Key Functions

ObjectID = type used to store id.

- `void addTag(ObjectID id, const std::string& tag);`
- `void removeTag(ObjectID id, const std::string& tag);`
- `bool hasTag(ObjectID, const std::string& tag) const;`
- `void removeObject(ObjectID id);`
- `std::vector<ObjectID> withTag(const std::string& tag) const;`
- `std::vector<ObjectID> withIncluded(vector<const std::string&> includeTags) const;`
- `std::vector<ObjectID> withIncludedExcluded(vector<const std::string& includeTags, excludeTags) const;`

## 4) Error Conditions

- Empty tag passed in
**Programmer error (assert)**
  
- Invalid ObjectID
**Programmer error (assert)**
  
- Memory allocation failure when inserting into maps/sets
**Recoverable (exceptions)**
  
- Querying a tag that doen not exist
**User error (return empty vector)**

## 5) Expected Challenges

It may become difficult if Nick and I do not plan together thoroughly before we begin and as we work on our classes. Avoiding the bug of index saying object has a tag while AnnotationSet says that it does not. Balancing simplicity with performance.

## 6) Coordination with Other Groups

List of other groups’ C++ classes to possibly would need to coordinate with:

- AnnotationSet
- DataMap
- BehaviorTree
- ActionMap


---
