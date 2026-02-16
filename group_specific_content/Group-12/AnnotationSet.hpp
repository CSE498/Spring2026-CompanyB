/**
 * @file AnnotationSet.hpp
 * @brief Declaration of the AnnotationSet class.
 * @version 1.0
 * @author Nicholas McComb
 * @date 2026-02-03
 */

#pragma once

#include <unordered_set> 
#include <string>

/*
* @brief Type definition for a tag.
*/
using tag = std::string;


/*
* @brief Class representing a set of annotations with associated tags.
*/
class AnnotationSet{ 

    private:

        /*
        * @brief A set of tags associated with the annotation set.
        */
        std::unordered_set<tag> tags;
    public:

        AnnotationSet() = default; // Default constructor
        ~AnnotationSet() = default; // Default destructor

        /*
        * @brief Adds a tag to the annotation set.
        * @param tag The tag to be added.
        */
        void addTag(const tag& tag);

        /*
        * @brief Removes a tag from the annotation set.
        * @param tag The tag to be removed.
        * @return true if the tag was removed, false if it was not found.
        */
        bool removeTag(const tag& tag);

        /*
        * @brief Checks if a tag exists in the annotation set. 
        * @param tag The tag to be checked.
        * @return true if the tag exists, false otherwise.
        */ 
        bool hasTag(const tag& tag) const;

        /*
        * @brief Retrieves a tag from the annotation set.
        * @param tag The tag to be retrieved.
        * @return A pointer to the tag if found, nullptr otherwise.
        */
        const tag* getTag(const tag& tag) const; 
        
        /*
        * @brief Retrieves all tags in the annotation set.
        * @return A set of all tags.
        */
        const std::unordered_set<tag>& getTags() const;

        /*
        * @brief Returns the number of tags in the annotation set.
        * @return The number of tags.
        */
        size_t size() const;

        /*
        * @brief Checks if the annotation set is empty.
        * @return true if the annotation set is empty, false otherwise.
        */
        bool empty() const;

        /*
        * @brief Clears all tags from the annotation set.
        */
        void clear();

     
};