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

std::optional<const tag*> AnnotationSet::addTag(const tag& tagToAdd) {
  auto [it, inserted] = tags.insert(tagToAdd);
  if (inserted) {
    return &*it;
  }
  return std::nullopt;
}

std::expected<void, std::string> AnnotationSet::removeTag(
    const tag& tagToRemove) {
  return tags.erase(tagToRemove) > 0 ? std::expected<void, std::string>{}
                                     : std::unexpected("Tag not found");
}

bool AnnotationSet::hasTag(const tag& queryTag) const {
  return tags.find(queryTag) != tags.end();
}

std::optional<const tag*> AnnotationSet::getTag(const tag& queryTag) const {
  auto it = tags.find(queryTag);
  if (it != tags.end()) {
    return &(*it);
  }
  return std::nullopt;
}

std::optional<const tag*> AnnotationSet::operator[](const tag& queryTag) const {
  return getTag(queryTag);
}

const std::unordered_set<tag>& AnnotationSet::getTags() const { return tags; }

size_t AnnotationSet::size() const { return tags.size(); }

bool AnnotationSet::empty() const { return tags.empty(); }

void AnnotationSet::clear() { tags.clear(); }

}  // namespace cse498