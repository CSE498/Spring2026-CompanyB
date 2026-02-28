// By Luke Bridges
#pragma once
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "Random.hpp"

namespace cse498 {
/*
  The initial boilerplate setup (constructors, CopyTree(), assign and
  move operators, basic getters (namely size(), empty(), total_weight()), and
  basic member variables) was written by Claude Code. I then modified many of
  those substantially as the design of the class changed; then they were further
  modified as part of Claude's other major contribution to this file, namely
  refactoring it to use std::unique_ptr for root_, left, and right. (As part of
  that refactor, it made a few minor unrelated changes, e.g. changing some
  spacing and using the default no-argument constructor.)

  Another Claude change, done near the end, was reorganizing the existing
  comments to use Doxygen formatting. Mostly this didn't change the content of
  the comments, only rearranged it using different delimiters + sections like
  @brief, @remark, etc., but it did add some extra boilerplate comments, I guess
  for completeness.

  All other AI contributions are marked with a comment near them.
*/
/**
 * @brief A weighted set data structure that supports weighted random selection.
 * @tparam T Arbitrary value type; should be hashable since there's an
 * unordered_map in here.
 *
 * @remark The idea of WeightedSet: can add elements with a "weight", then
 * randomly select them with probability proportional to the weight. E.g. if it
 * contains one element of weight 1 and one of weight 2, the first will be
 * selected with 1/3 probability and the second with 2/3 probability.
 */
template <typename T>
class WeightedSet {
 private:
  /** @brief A node in the binary-tree structure used in WeightedSet. */
  struct Node {
    /**
     * @brief Pointer to the value "contained" in the node.
     * @remark Note on the implementation: the value itself is owned and stored
     * in the unordered_map used in WeightedSet--we need to have the values as
     * keys in the unordered_map so we can use it for membership checking, and
     * once we've stored the value there, it would be a waste to duplicate it in
     * the node.
     */
    const T* value_ptr;
    double weight;          ///< This element's weight (nonnegative)
    double subtree_weight;  ///< Total weight of subtree rooted at this node
    std::unique_ptr<Node> left;   ///< Left child (owning)
    std::unique_ptr<Node> right;  ///< Right child (owning)
    Node* parent;                 ///< Parent (non-owning)

    /**
     * @brief Constructs a new node.
     * @param val_ptr Pointer to the value.
     * @param w Weight of the element.
     * @param par Parent node (default: nullptr).
     */
    Node(const T* val_ptr, double w, Node* par = nullptr)
        : value_ptr(val_ptr),
          weight(w),
          subtree_weight(w),
          left(nullptr),
          right(nullptr),
          parent(par) {}
  };

  std::unique_ptr<Node> root_;  ///< Root of the tree
  std::random_device rd_{};  ///< Random device.

  /**
   * @brief Xoshiro random number generator.
   * @remark Marked as mutable because its state gets updated whenever we
   * generate a random number, including in GetRandomElement which is a const
   * function.
   */
  mutable cse498::Random rng_{rd_()};
  /**
   * @brief Maps elements to their nodes in the tree.
   * @remark Keys are elements stored in the WeightedSet; values are non-owning
   *         pointers to the nodes in the tree representing that element.
   */
  std::unordered_map<T, Node*> element_to_node_;

  /** @brief Check if a node is a leaf (has no children). */
  bool IsLeaf(Node* node) const {
    assert(node != nullptr);
    return node->left == nullptr && node->right == nullptr;
  }

  /**
   * @brief Returns a reference to the unique_ptr that owns the given node.
   * @param node The node whose owning pointer to retrieve.
   * @return Reference to the unique_ptr that owns the node (root_ or a parent's
   * left/right child pointer).
   * @remark Written by Claude as part of a refactor to use unique_ptr.
   */
  std::unique_ptr<Node>& OwningPointer(Node* node) {
    if (node->parent == nullptr) return root_;
    if (node->parent->left.get() == node) return node->parent->left;
    return node->parent->right;
  }
  /**
   * @brief Copies the subtree starting at src, making it a child of the given
   * "parent" node.
   * @param src The root of the subtree to copy.
   * @param parent The parent node for the copied subtree.
   * @return A unique pointer to the copy of src it creates.
   */
  std::unique_ptr<Node> CopyTree(const Node* src, Node* parent) {
    if (!src) return nullptr;

    auto element = *(src->value_ptr);
    auto [iter, inserted] = element_to_node_.insert({element, nullptr});
    const T* element_ptr = &(iter->first);
    auto node = std::make_unique<Node>(element_ptr, src->weight, parent);
    iter->second = node.get();

    node->subtree_weight = src->subtree_weight;
    node->left = CopyTree(src->left.get(), node.get());
    node->right = CopyTree(src->right.get(), node.get());
    return node;
  }

  /** @brief Returns the total weight of the left subtree, or 0 if there is no
   * left child. */
  double LeftSubtreeWeight(const Node* const node) const {
    return node->left ? node->left->subtree_weight : 0.0;
  }

  /** @brief Returns the total weight of the right subtree, or 0 if there is no
   * right child. */
  double RightSubtreeWeight(const Node* const node) const {
    return node->right ? node->right->subtree_weight : 0.0;
  }
  /**
   * @brief Ensures that a given node and all its ancestors have their total
   * subtree weight recorded correctly.
   * @param node_ptr The lowest node that had its subtree change during an
   * insertion or deletion.
   * @remark Generally you pass in the lowest node that had its subtree change
   * during an insertion or deletion: then it and all of its ancestors may also
   * have had their subtree weights change as a result.
   */
  void FixWeightsAndRebalance(Node* node_ptr) {
    while (node_ptr != nullptr) {
      // TODO: rotate if doing so would make the tree better weight-balanced
      node_ptr->subtree_weight = node_ptr->weight +
                                 LeftSubtreeWeight(node_ptr) +
                                 RightSubtreeWeight(node_ptr);
      node_ptr = node_ptr->parent;
    }
  }

  /**
   * @brief Helper for DebugPrint: recursively builds tree visualization.
   * @param os Output stream to write to.
   * @param node Current node being printed.
   * @param prefix String prefix for indentation.
   * @param is_left Whether this node is a left child.
   * @param is_root Whether this node is the root.
   * @remark Claude-written.
   */
  void DebugPrintNode(std::ostream& os, const Node* node,
                      const std::string& prefix, bool is_left,
                      bool is_root) const {
    if (!node) return;

    os << prefix;
    if (is_root) {
      os << "";
    } else {
      os << (is_left ? "├── L: " : "└── R: ");
    }

    os << "[" << *(node->value_ptr) << " w=" << node->weight
       << " sw=" << node->subtree_weight << "]" << std::endl;

    std::string child_prefix =
        prefix + (is_root ? "" : (is_left ? "│   " : "    "));

    if (node->left || node->right) {
      DebugPrintNode(os, node->left.get(), child_prefix, true, false);
      DebugPrintNode(os, node->right.get(), child_prefix, false, false);
    }
  }

  /** @brief Check if child is a direct child of parent (left or right). */
  bool IsDirectChild(const Node* const parent, const Node* const child) const {
    return (parent->left.get() == child) || (parent->right.get() == child);
  }

  /**
   * @brief Searches for a child of the given node which is a leaf.
   * @param current The node to start searching from.
   * @return A pointer to a leaf node in the subtree.
   * @remark Such a child exists in any tree, as long as the given node has at
   * least one child. Favors the heavier subtree, which should help Remove
   * balance the tree.
   */
  Node* FindLeaf(Node* const current) const {
    if (IsLeaf(current)) {
      return current;
    }
    if (LeftSubtreeWeight(current) > RightSubtreeWeight(current)) {
      return FindLeaf(current->left.get());
    } else {
      return FindLeaf(current->right.get());
    }
  }

  /**
   * @brief Helper function for "Remove". Given a node to_delete, swap in
   * "replacement".
   * @param to_delete The node to be deleted.
   * @param replacement The node to replace it with (a direct child and/or a
   * leaf).
   * @remark The replacement is a direct child (in the case where to_delete has
   * 1 child) and/or a leaf (in the case where to_delete has 2 children).
   * @remark Commenting this more heavily than usual because otherwise it can be
   * a bit unclear what's going on here, IMO
   * @remark This function was revised especially heavily by Claude as part of a
   * refactor to use unique_ptr for "left", "right", and "root_"
   */
  void ReplaceDeletedNode(Node* to_delete, Node* replacement) {
    assert(to_delete != nullptr);
    assert(replacement != nullptr);
    assert(
        (to_delete->left && to_delete->right && IsLeaf(replacement)) ||
        (to_delete->left.get() == replacement && to_delete->right == nullptr) ||
        (to_delete->right.get() == replacement && to_delete->left == nullptr));

    // Step 1: Detach the replacement from its current parent's ownership
    auto replacement_owned = std::move(OwningPointer(replacement));

    // Step 2: Transfer children from the deleted node to the replacement.
    // Skip if the replacement IS that child (to avoid a self-referencing loop).
    if (to_delete->left && replacement != to_delete->left.get()) {
      replacement_owned->left = std::move(to_delete->left);
    }
    if (to_delete->right && replacement != to_delete->right.get()) {
      replacement_owned->right = std::move(to_delete->right);
    }

    // Step 3: Update parent pointers of the transferred children
    if (replacement_owned->left) {
      replacement_owned->left->parent = replacement_owned.get();
    }
    if (replacement_owned->right) {
      replacement_owned->right->parent = replacement_owned.get();
    }

    // Step 4: Update the replacement's parent to be the deleted node's parent
    replacement_owned->parent = to_delete->parent;

    // Step 5: Place the replacement into the deleted node's position in the
    // tree. This assignment destroys to_delete via unique_ptr; its children
    // have already been moved out above, so they survive.
    OwningPointer(to_delete) = std::move(replacement_owned);
  }

 public:
  /** @brief Default constructor. */
  WeightedSet() = default;

  /** @brief Destructor. */
  ~WeightedSet() = default;

  /** @brief Copy constructor. */
  WeightedSet(const WeightedSet& other) {
    root_ = CopyTree(other.root_.get(), nullptr);
  }

  /** @brief Move constructor. */
  WeightedSet(WeightedSet&& other) noexcept
      : root_(std::move(other.root_)),
        element_to_node_(std::move(other.element_to_node_)) {}

  /** @brief Copy assignment operator. */
  WeightedSet& operator=(const WeightedSet& other) {
    if (this != &other) {
      root_.reset();
      element_to_node_.clear();
      root_ = CopyTree(other.root_.get(), nullptr);
    }
    return *this;
  }

  /** @brief Move assignment operator. */
  WeightedSet& operator=(WeightedSet&& other) noexcept {
    if (this != &other) {
      root_ = std::move(other.root_);
      element_to_node_ = std::move(other.element_to_node_);
    }
    return *this;
  }
  /**
   * @brief Insert an element into the tree with a given weight.
   * @param element The element to insert.
   * @param weight The weight of the element (must be positive).
   * @return Whether the insertion is successful.
   * @remark If the element is already there, we simply update its weight. When
   * inserting a new element, we (recursively) put it in the subtree which
   * currently has the lightest weight, to keep the tree weight-balanced (which
   * should theoretically improve the speed of the GetRandomElement function).
   * The insertion fails if the weight is invalid (it must be positive) or if
   * the function somehow fails to find a place to insert the element.
   */
  bool Insert(const T& element, const double weight) {
    if (weight <= 0) {
      return false;
    }
    auto it = element_to_node_.find(element);
    if (it != element_to_node_.end()) {
      auto node_ptr = it->second;
      node_ptr->weight = weight;
      FixWeightsAndRebalance(node_ptr);
      return true;
    }
    /**
     * Note: this code, to obtain a pointer to the T element stored in the map,
     * was modified from a snippet by Claude. It replaces an older version that
     * created two copies of the element, one stored in the map and one pointed
     * to by the node's value_ptr
     */
    auto [iter, inserted] = element_to_node_.insert({element, nullptr});
    const T* element_ptr = &(iter->first);
    auto node = std::make_unique<Node>(element_ptr, weight);
    Node* node_raw = node.get();
    iter->second = node_raw;

    if (!root_) {
      root_ = std::move(node);
      return true;
    }
    Node* current_node = root_.get();
    while (current_node != nullptr) {
      // if a node has less than 2 children, we can just insert the new node in
      // one of the empty spaces.
      if (!current_node->left) {
        node_raw->parent = current_node;
        current_node->left = std::move(node);
        FixWeightsAndRebalance(current_node);
        return true;
      } else if (!current_node->right) {
        node_raw->parent = current_node;
        current_node->right = std::move(node);
        FixWeightsAndRebalance(current_node);
        return true;
        // if a node does have 2 children, we insert the new node in the lighter
        // of the two subtrees.
      } else if (LeftSubtreeWeight(current_node) <
                 RightSubtreeWeight(current_node)) {
        current_node = current_node->left.get();
      } else {
        current_node = current_node->right.get();
      }
    }
    return false;
  }

  /**
   * @brief Takes in an index in the range [0, total weight) and returns the
   * element "at" that index.
   * @param index The index to look up (must be in [0, total_weight())).
   * @return The element at the given index, or std::nullopt if the index is out
   * of range.
   * @remark Each element has an interval associated to it, with length equal to
   * that element's weight, such that, if you give an index in that interval,
   * you'll get back that element. E.g. if you have just 1 element of weight 1
   * and another of weight 2, then (depending on the order in which they were
   * inserted) either the weight-1 element has [0, 1) and the weight-2 has [1,
   * 3) or the weight-1 has [2, 3) and the weight-2 has [0, 2). Classes other
   * than WeightedSet shouldn't worry, or need to worry, about the exact
   * interval assigned to each element. These intervals may change on inserting
   *         or deleting elements. The only guarantee is that the length of the
   * interval will be equal to the weight of the element--which is needed for
   * GetRandomElement to work properly.
   */
  std::optional<T> GetElementAt(const double index) const {
    Node* current_node = root_.get();
    double total = total_weight();
    if (index < 0 || index >= total) {
      return std::nullopt;
    }
    double lower_bound = 0;
    while (current_node != nullptr) {
      /**
       * On the first iteration of this loop, we divide the whole interval [0,
       * total weight] into 3 subintervals: [0, left subtree weight), [left
       * subtree weight, left subtree weight + root weight), and [left subtree
       * weight + root weight, total weight). Note that the middle interval has
       * length equal to the weight of the root. If the index is in the middle
       * interval, we just return the root. If in the first interval: we enter
       * the left subtree and continue from there, dividing up that interval in
       * the same way. Same idea for the third interval and the right subtree.
       */
      double current_node_interval_lower =
          lower_bound + LeftSubtreeWeight(current_node);
      double current_node_interval_upper =
          current_node_interval_lower + current_node->weight;
      if (index < current_node_interval_lower) {
        current_node = current_node->left.get();
      } else if (index >= current_node_interval_upper) {
        lower_bound = current_node_interval_upper;
        current_node = current_node->right.get();
      } else {
        return std::make_optional(*(current_node->value_ptr));
      }
    }
    return std::nullopt;
  }

  /**
   * @brief Returns an element chosen at random.
   * @return A randomly chosen element, or std::nullopt if the set is empty.
   * @remark Each element's probability of being chosen is (weight of element) /
   * (total weight).
   */
  std::optional<T> GetRandomElement() const {
    if (!root_) {
      return std::nullopt;
    }
    return GetElementAt(rng_.nextDouble(0, total_weight()));
  }

  /**
   * @brief Removes the given element from the set.
   * @param element The element to remove.
   * @return The element (if it was removed successfully) or nullopt (if the
   * element isn't in the set or the removal otherwise failed).
   */
  std::optional<T> Remove(const T& element) {
    auto it = element_to_node_.find(element);
    if (it == element_to_node_.end()) {
      return std::nullopt;
    }
    Node* node_ptr = it->second;
    T removed_value = *(node_ptr->value_ptr);

    Node* to_rebalance = node_ptr->parent;
    if (!node_ptr->left && !node_ptr->right) {
      // Leaf node: just remove it. This destroys node_ptr.
      OwningPointer(node_ptr).reset();
    } else if (!node_ptr->left) {
      to_rebalance = node_ptr->right.get();
      ReplaceDeletedNode(node_ptr, node_ptr->right.get());
    } else if (!node_ptr->right) {
      to_rebalance = node_ptr->left.get();
      ReplaceDeletedNode(node_ptr, node_ptr->left.get());
    } else {
      Node* replacement = FindLeaf(node_ptr->left.get());
      /**
       * A subtle point: if the replacement node is a direct child of the node
       * being replaced, then the lowest node affected by the change will be the
       * replacement, and the replacement's parent will be gone soon. Otherwise,
       * the lowest affected node is the replacement's parent, which lost a
       * child but is still in the tree.
       */
      to_rebalance = IsDirectChild(node_ptr, replacement) ? replacement
                                                          : replacement->parent;
      ReplaceDeletedNode(node_ptr, replacement);
    }
    FixWeightsAndRebalance(to_rebalance);
    /**
     * node_ptr has already been destroyed by the unique_ptr operations above;
     * just clean up the map entry.
     */
    element_to_node_.erase(element);
    return std::make_optional(removed_value);
  }

  /** @brief Returns the total number of elements in the set. */
  size_t size() const { return element_to_node_.size(); }

  /** @brief Returns whether the set is empty. */
  bool empty() const { return size() == 0; }

  /** @brief Returns the total weight of all the elements in the set. */
  double total_weight() const { return root_ ? root_->subtree_weight : 0.0; }

  /**
   * @brief Returns whether the set contains the given element.
   * @param element The element to check for.
   * @return True if the element is in the set, false otherwise.
   */
  bool Contains(const T& element) const {
    return element_to_node_.find(element) != element_to_node_.end();
  }

  /**
   * @brief Visualization: prints ASCII tree to ostream.
   * @param os Output stream to print to (default: cout).
   * @remark Claude-written.
   */
  void DebugPrint(std::ostream& os = std::cout) const {
    if (!root_) {
      os << "(empty tree)" << std::endl;
      return;
    }
    DebugPrintNode(os, root_.get(), "", false, true);
  }

  /**
   * @brief Return tree as string instead of printing.
   * @return String representation of the tree.
   * @remark Claude-written.
   */
  std::string DebugString() const {
    std::ostringstream oss;
    DebugPrint(oss);
    return oss.str();
  }
};

}  // namespace cse498
