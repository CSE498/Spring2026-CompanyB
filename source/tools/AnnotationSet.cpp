/**
 * @file AnnotationSet.cpp
 * @brief Implementation of the AnnotationSet class methods.
 * @version 1.0
 * @author Nicholas McComb
 * @date 2026-02-03
 */

#include "AnnotationSet.hpp"


namespace cse498 {
/**
 * @brief All AnnotationSet class methods.
 */

void AnnotationSet::addTag(const tag& tagToAdd){
    tags.insert(tagToAdd);
}

bool AnnotationSet::removeTag(const tag& tagToRemove){
    return tags.erase(tagToRemove) > 0;
}

bool AnnotationSet::hasTag(const tag& queryTag) const{
    return tags.find(queryTag) != tags.end();
}

std::optional<const tag *> AnnotationSet::getTag(const tag& queryTag) const{
    auto it = tags.find(queryTag);
    if(it != tags.end()){
        return &(*it);
    }
    return std::nullopt;
}

std::optional<const tag *> AnnotationSet::operator[](const tag& queryTag) const{
    auto it = tags.find(queryTag);
    if(it != tags.end()){
        return &(*it);
    }
    return std::nullopt;
}

const std::unordered_set<tag>& AnnotationSet::getTags() const{
    return tags;
}

size_t AnnotationSet::size() const{
    return tags.size();
}

bool AnnotationSet::empty() const{
    return tags.empty();
}

void AnnotationSet::clear(){
    tags.clear();
}

} // cse498 namespace