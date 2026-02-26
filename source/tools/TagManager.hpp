// Author: Landon Cosby
//
// Brief Description: Header file for the TagManager class, which manages
// the registration of objects and their associated tags, as well as querying.

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "AnnotationSet.hpp"

// TagManager maintains a bidirectional mapping between object IDs and their associated tags,
// allowing for efficient registration, unregistration, and querying of objects based on their tags.
// It ensures that the internal state remains consistent when objects are modified or removed,
// and provides helper functions for set operations during queries.
class TagManager {
    public:

        // Unique identifier for objects being tagged.
        using ObjectId = std::uint64_t;

        TagManager() = default;
        ~TagManager() = default;

        // Registers the object, indexes all the tags that are in the AnnotationSet.
        // If the object_id is already registered, it will be unregistered first to prevent duplicates.
        void RegisterObject(ObjectId object_id, const cse498::AnnotationSet& set);

        // Unregisters the object, removing it from all tag buckets and deleting its entry in object_tags.
        bool UnregisterObject(ObjectId object_id);

        // Adds a tag to the object's AnnotationSet and updates the index.
        // If the object is not registered, it will be registered first.
        bool AddTag(ObjectId object_id, cse498::AnnotationSet& set, const cse498::tag& tag);

        // Removes a tag from the given object and updates the indexes.
        bool RemoveTag(ObjectId object_id, cse498::AnnotationSet& set, const cse498::tag& tag);

        // Clears all tags from the object's AnnotationSet and updates the index accordingly.
        void ClearTags(ObjectId object_id, cse498::AnnotationSet& set);

        // Returns a list of object_ids that have all the tags in must_have and none of the tags in must_not_have.
        std::vector<ObjectId> QueryMultiTags(const std::vector<cse498::tag>& must_have, const std::vector<cse498::tag>& must_not_have = {}) const;

        // Returns a list of object_ids that have the specified tag.
        std::vector<ObjectId> QueryOneTag(const cse498::tag& tag) const;

        // Checks if an object_id is registered.
        bool IsRegistered(ObjectId object_id) const;

        private:
        // tag_index: tag -> set of object_ids, useful for the QueryOneTag function
        std::unordered_map<cse498::tag, std::unordered_set<ObjectId>> tag_index;

        // object_tags: object_id -> set of tags, useful for the QueryMultiTags function
        std::unordered_map<ObjectId, std::unordered_set<cse498::tag>> object_tags;

        // Helper function to perform AND operation on result set with another set of object_ids.
        static void AndQueryHelper(std::unordered_set<ObjectId>& result, const std::unordered_set<ObjectId>& other);

        // Helper function to perform NOT operation on result set with another set of object_ids.
        static void NotQueryHelper(std::unordered_set<ObjectId>& result, const std::unordered_set<ObjectId>& remove);

};