// Author: Landon Cosby
//
// Brief Description: Implementation of the TagManager class, which manages
// the registration of objects and their associated tags, as well as querying
// objects based on their tags.

 #include "TagManager.hpp"


  // Registers the object, indexes all the tags that are in the AnnotationSet.
  // If the object_id is already registered, it will be unregistered first to prevent duplicates.
  void TagManager::RegisterObject(ObjectId object_id, const cse498::AnnotationSet& set) {
    // If already registered, unregister first to prevent duplicates
    UnregisterObject(object_id);

    // tag_index: tag -> set of object_ids
    // object_tags: object_id -> set of tags
    for (const auto& tag : set.getTags()) {
        tag_index[tag].insert(object_id);
        object_tags[object_id].insert(tag);
    }
  }

  // Unregisters the object, removing it from all tag buckets and deleting its entry in object_tags.
  // Returns true if the object was found and unregistered, false if it was not found.
  bool TagManager::UnregisterObject(ObjectId object_id) {
    auto it = object_tags.find(object_id);
    if (it == object_tags.end()) {
        return false;
    }

    // Remove object_id from all tag buckets in tag_index and clean up empty buckets.
    for (const auto& tag : it->second) {
        tag_index[tag].erase(object_id);
        if (tag_index[tag].empty()) {
            tag_index.erase(tag);
        }
    }

    object_tags.erase(it);
    return true;
  }

  // Adds a tag to the object's AnnotationSet and updates the index. If the object is not registered, it will be registered first.
  // Returns true if the tag was added, false if the tag already existed in the set.
  bool TagManager::AddTag(ObjectId object_id, cse498::AnnotationSet& set, const cse498::tag& tag) {
    if (!IsRegistered(object_id)) {
        RegisterObject(object_id, set);
    }

    if (set.hasTag(tag)) {
        return false;
    }

    set.addTag(tag);
    tag_index[tag].insert(object_id);
    object_tags[object_id].insert(tag);
    return true;
  }

  // Removes a tag from the object's AnnotationSet and updates the index.
  // Returns true if the tag was removed, false if the tag was not found in the set.
  bool TagManager::RemoveTag(ObjectId object_id, cse498::AnnotationSet& set, const cse498::tag& tag) {
    if (!set.hasTag(tag)) return false;

    set.removeTag(tag);

    // Remove object_id from the tag bucket in tag_index and clean up if empty
    auto ti = tag_index.find(tag);
    if (ti != tag_index.end()) {
        ti->second.erase(object_id);
        if (ti->second.empty()) tag_index.erase(ti);
    }

    // Remove tag from the object's tag set in object_tags and clean up if empty
    auto oi = object_tags.find(object_id);
    if (oi != object_tags.end()) {
        oi->second.erase(tag);
        if (oi->second.empty()) object_tags.erase(oi);
    }

    return true;
  }

  // Clears all tags from the object's AnnotationSet and updates the index accordingly.
  // If the object is not registered, it will simply clear the provided set.
  void TagManager::ClearTags(ObjectId object_id, cse498::AnnotationSet& set) {
    auto it = object_tags.find(object_id);
    if (it != object_tags.end()) {
        for (const auto& t : it->second) {
            auto ti = tag_index.find(t);
            if (ti != tag_index.end()) {
                ti->second.erase(object_id);
                if (ti->second.empty()) tag_index.erase(ti);
            }
        }
        object_tags.erase(it);
    }
    set.clear();
  }

  // Helper function to perform AND operation on result set with another set of object_ids.
  void TagManager::AndQueryHelper(std::unordered_set<ObjectId>& result, const std::unordered_set<ObjectId>& other) {
    for (auto i = result.begin(); i != result.end();) {
        if (other.find(*i) == other.end()) {
            i = result.erase(i);
        } else {
            ++i;
        }
    }
  }

    // Helper function to perform NOT operation on result set with another set of object_ids.
    void TagManager::NotQueryHelper(std::unordered_set<ObjectId>& result, const std::unordered_set<ObjectId>& remove) {
        for (const auto& id : remove) {
            result.erase(id);
        }
    }

    // Checks if an object_id is registered.
    bool TagManager::IsRegistered(ObjectId object_id) const {
        return object_tags.find(object_id) != object_tags.end();
    }

    // Returns a list of object_ids that have the specified tag.
    std::vector<TagManager::ObjectId> TagManager::QueryOneTag(const cse498::tag& tag) const {
        std::vector<ObjectId> result;
        auto it = tag_index.find(tag);

        if (it != tag_index.end()) {
            result.insert(result.end(), it->second.begin(), it->second.end());
        }
        return result;
    }

    // Returns a list of object_ids that have all the tags in must_have and none of the tags in must_not_have.
    std::vector<TagManager::ObjectId> TagManager::QueryMultiTags(const std::vector<cse498::tag>& must_have, const std::vector<cse498::tag>& must_not_have) const {
        if (must_have.empty() && must_not_have.empty()) {
            return {};
        }

        std::unordered_set<ObjectId> result;

        // Start with the set of object_ids for the first required tag, then AND with the rest.
        //If must_have is empty, start with an empty result and only apply NOT filters.
        if (!must_have.empty()) {
            auto it = tag_index.find(must_have[0]);
            if (it == tag_index.end()) {
                return {};
            }
            result = it->second;

            // AND with the rest of the required tags
            for (size_t i = 1; i < must_have.size(); ++i) {
                it = tag_index.find(must_have[i]);
                if (it == tag_index.end()) {
                    return {};
                }
                AndQueryHelper(result, it->second);
                if (result.empty()) {
                    return {};
                }
            }
        }

        // Apply NOT filters
        for (const auto& tag : must_not_have) {
            auto it = tag_index.find(tag);
            if (it != tag_index.end()) {
                NotQueryHelper(result, it->second);
                if (result.empty()) {
                    return {};
                }
            }
        }

        return std::vector<ObjectId>(result.begin(), result.end());
    }
