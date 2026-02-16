/**
 * @file AnnotationSet.cpp
 * @brief Implementation of the AnnotationSet class methods.
 * @version 1.0
 * @author Nicholas McComb
 * @date 2026-02-03
 */

#include "AnnotationSet.hpp"

/**
 * @brief All AnnotationSet class methods.
 */

void AnnotationSet::addTag(const tag& tag){
    tags.insert(tag);
}

bool AnnotationSet::removeTag(const tag& tag){
    return tags.erase(tag) > 0;
}

bool AnnotationSet::hasTag(const tag& tag) const{
    return tags.find(tag) != tags.end();
};

const tag* AnnotationSet::getTag(const tag& tag) const{
    auto it = tags.find(tag);
    if(it != tags.end()){
        return &(*it);
    }
    return nullptr;
};

const std::unordered_set<tag>& AnnotationSet::getTags() const{
    return tags;
};

size_t AnnotationSet::size() const{
    return tags.size();
};

bool AnnotationSet::empty() const{
    return tags.empty();
};

void AnnotationSet::clear(){
    tags.clear();
};
