#pragma once
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace cse498 {
/*
  The initial boilerplate setup (constructors, CopyTree(), assign and
  move operators, basic getters (namely size(), empty(), total_weight()), and
  basic member variables) was written by Claude Code. I then modified many of those
  substantially as the design of the class changed; then they were further modified
  as part of Claude's other major contribution to this file, namely refactoring it to use
  std::unique_ptr for root_, left, and right. (As part of that refactor, it made a few minor
  unrelated changes, e.g. changing some spacing and using the default no-argument constructor.)

  All other AI contributions are marked with a comment near them.
*/
// T = arbitrary value type; should be hashable I guess since there's an
// unordered_map in here
template <typename T>
/*
  The idea of WeightedSet: can add elements with a "weight", then randomly
  select them with probability proportional to the weight. E.g. if it contains
  one element of weight 1 and one of weight 2, the first will be selected with
  1/3 probability and the second with 2/3 probability.
*/
class WeightedSet {
 private:
  // A node in the binary-tree structure used in WeightedSet.
  struct Node {
    // Pointer to the value "contained" in the node.
    // Note on the implementation: the value itself is owned and stored in the
    // unordered_map used in WeightedSet--we need to have the values as keys in
    // the unordered_map so we can use it for membership checking, and once
    // we've stored the value there, it would be a waste to duplicate it in the
    // node.
    const T* value_ptr;
    double weight;                // this element's weight (nonnegative)
    double subtree_weight;        // total weight of subtree rooted at this node
    std::unique_ptr<Node> left;   // left child (owning)
    std::unique_ptr<Node> right;  // right child (owning)
    Node* parent;                 // parent (non-owning)

    Node(const T* val_ptr, double w, Node* par = nullptr)
        : value_ptr(val_ptr),
          weight(w),
          subtree_weight(w),
          left(nullptr),
          right(nullptr),
          parent(par) {}
  };

  std::unique_ptr<Node> root_;
  // TODO: replace this with custom random class once we have that
  std::random_device rd_{};
  mutable std::mt19937 rng_{rd_()};
  // Keys are elements stored in the WeightedSet; values are non-owning
  // pointers to the nodes in the tree representing that element.
  std::unordered_map<T, Node*> element_to_node_;

  // Written by Claude as part of a refactor to use unique_ptr.
  // Returns a reference to the unique_ptr that owns the given node,
  // whether that's root_ or a parent's left/right child pointer.
  std::unique_ptr<Node>& OwningPointer(Node* node) {
    if (node->parent == nullptr) return root_;
    if (node->parent->left.get() == node) return node->parent->left;
    return node->parent->right;
  }

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

  double LeftSubtreeWeight(const Node* const node) const {
    return node->left ? node->left->subtree_weight : 0.0;
  }

  double RightSubtreeWeight(const Node* const node) const {
    return node->right ? node->right->subtree_weight : 0.0;
  }
  // Ensures that a given node and all its ancestors have their total subtree
  // weight recorded correctly. Generally you pass in the lowest node that had
  // its subtree change during an insertion or deletion: then it and all of its
  // ancestors may also have had their subtree weights change as a result.
  void FixWeightsAndRebalance(Node* node_ptr) {
    while (node_ptr != nullptr) {
      // TODO: rotate if doing so would make the tree better weight-balanced
      node_ptr->subtree_weight = node_ptr->weight +
                                 LeftSubtreeWeight(node_ptr) +
                                 RightSubtreeWeight(node_ptr);
      node_ptr = node_ptr->parent;
    }
  }

  // (Claude-written) helper for DebugPrint: recursively builds tree
  // visualization
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

  bool IsDirectChild(const Node* const parent, const Node* const child) const {
    return (parent->left.get() == child) || (parent->right.get() == child);
  }

  // Replaces to_delete in the tree with replacement (a child already released
  // from to_delete). Destroys to_delete via its owning unique_ptr.
  void ReplaceDeletedNodeWithChild(Node* to_delete,
                                   std::unique_ptr<Node> replacement) {
    if (replacement) {
      replacement->parent = to_delete->parent;
    }
    // Assigning to the owning pointer destroys to_delete; its children must
    // already have been moved out so they aren't destroyed along with it.
    OwningPointer(to_delete) = std::move(replacement);
  }

  // Searches for a child of the given node which is a leaf (such a child exists
  // in any tree, as long as the given node has at least one child).
  Node* FindLeaf(Node* const current) const {
    if (!current->left && !current->right) {
      return current;
    }
    if (current->left) {
      return FindLeaf(current->left.get());
    } else {
      return FindLeaf(current->right.get());
    }
  }
  // we take a leaf node and swap it in for the node to be deleted.
  // the leaf's left, right, and parent should become the left, right, and
  // parent of the deleted node. then we need to rebalance starting at the
  // previous parent of the leaf (if that isn't the deleted node) or starting at
  // the moved leaf (if the leaf was a child of the deleted node).

  void ReplaceDeletedNodeWithLeaf(Node* to_delete, Node* leaf) {
    assert(to_delete != nullptr);
    assert(leaf != nullptr);
    // Commenting this more heavily than usual because otherwise it can be a bit
    // unclear what's going on here, IMO

    // Also, note: this function was revised especially heavily by Claude as
    // part of a refactor to use unique_ptr for "left", "right", and "root_"

    // Step 1: Detach the leaf from its current parent's ownership
    auto leaf_owned = std::move(OwningPointer(leaf));

    // Step 2: Transfer children from the deleted node to the leaf.
    // Skip if the leaf IS that child (to avoid a self-referencing loop).
    if (leaf != to_delete->left.get()) {
      leaf_owned->left = std::move(to_delete->left);
    }
    if (leaf != to_delete->right.get()) {
      leaf_owned->right = std::move(to_delete->right);
    }

    // Step 3: Update parent pointers of the transferred children
    if (leaf_owned->left) {
      leaf_owned->left->parent = leaf_owned.get();
    }
    if (leaf_owned->right) {
      leaf_owned->right->parent = leaf_owned.get();
    }

    // Step 4: Update the leaf's parent to be the deleted node's parent
    leaf_owned->parent = to_delete->parent;

    // Step 5: Place the leaf into the deleted node's position in the tree.
    // This assignment destroys to_delete via unique_ptr; its children have
    // already been moved out above, so they survive.
    OwningPointer(to_delete) = std::move(leaf_owned);
  }

 public:
  WeightedSet() = default;

  WeightedSet(const WeightedSet& other) {
    root_ = CopyTree(other.root_.get(), nullptr);
  }

  WeightedSet(WeightedSet&& other) noexcept
      : root_(std::move(other.root_)),
        element_to_node_(std::move(other.element_to_node_)) {}

  WeightedSet& operator=(const WeightedSet& other) {
    if (this != &other) {
      root_.reset();
      element_to_node_.clear();
      root_ = CopyTree(other.root_.get(), nullptr);
    }
    return *this;
  }

  WeightedSet& operator=(WeightedSet&& other) noexcept {
    if (this != &other) {
      root_ = std::move(other.root_);
      element_to_node_ = std::move(other.element_to_node_);
    }
    return *this;
  }
  // Insert an element into the tree with a given weight. Returns whether the
  // insertion is successful. If the element is already there, we simply update
  // its weight. When inserting a new element, we (recursively) put it in the
  // subtree which currently has the lightest weight, to keep the tree
  // weight-balanced (which should theoretically improve the speed of the
  // GetRandomElement function). The insertion fails if the weight is invalid
  // (it must be positive) or if the function somehow fails to find a place to
  // insert the element.
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
    // note: this code, to obtain a pointer to the T element stored in the map,
    // was modified from a snippet by Claude. It replaces an older version that
    // created two copies of the element, one stored in the map and one pointed
    // to by the node's value_ptr
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
  // This function takes in an index in the range [0, total weight] and returns
  // the element "at" that index. Each element has an interval associated to it,
  // with length equal to that element's weight, such that, if you give an index
  // in that interval, you'll get back that element. E.g. if you have just 1
  // element of weight 1 and another of weight 2, then (depending on the order
  // in which they were inserted) either the weight-1 element has [0, 1) and the
  // weight-2 has [1, 3] or the weight-1 has [2, 3] and the weight-2 has [0, 2).
  // Classes other than WeightedSet shouldn't worry, or need to worry, about the
  // exact interval assigned to each element. These intervals may change on
  // inserting or deleting elements. The only guarantee is that the length of
  // the interval will be equal to the weight of the element--which is needed
  // for GetRandomElement to work properly.
  std::optional<T> GetElementAt(const double index) const {
    Node* current_node = root_.get();
    double total = total_weight();
    if (index < 0 || index > total) {
      return std::nullopt;
    }
    double lower_bound = 0;
    double upper_bound = total;
    while (current_node != nullptr) {
      // On the first iteration of this loop, we divide the whole interval [0,
      // total weight] into 3 subintervals: [0, left subtree weight), [left
      // subtree weight, left subtree weight + root weight), and [left subtree
      // weight + root weight, total weight]. Note that the middle interval has
      // length equal to the weight of the root. If the index is in the middle
      // interval, we just return the root. If in the first interval: we enter
      // the left subtree and continue from there, dividing up that interval in
      // the same way. Same idea for the third interval and the right subtree.
      double current_node_interval_lower =
          lower_bound + LeftSubtreeWeight(current_node);
      double current_node_interval_upper =
          current_node_interval_lower + current_node->weight;
      if (index < current_node_interval_lower) {
        upper_bound = current_node_interval_lower;
        current_node = current_node->left.get();
      } else if (index >= current_node_interval_upper &&
                 current_node_interval_upper != upper_bound) {
        lower_bound = current_node_interval_upper;
        current_node = current_node->right.get();
      } else {
        return std::make_optional(*(current_node->value_ptr));
      }
    }
    return std::nullopt;
  }

  std::optional<T> GetRandomElement() const {
    if (!root_) {
      return std::nullopt;
    }
    auto random_real =
        std::uniform_real_distribution<double>(0, total_weight());
    return GetElementAt(random_real(rng_));
  }
  // Removes the given element from the set. Returns the element (if it was
  // removed successfully) or nullopt (if the element isn't in the set or the 
  // removal otherwise failed).
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
      // Release the right child, then replace node_ptr with it.
      ReplaceDeletedNodeWithChild(node_ptr, std::move(node_ptr->right));
    } else if (!node_ptr->right) {
      to_rebalance = node_ptr->left.get();
      ReplaceDeletedNodeWithChild(node_ptr, std::move(node_ptr->left));
    } else {
      Node* replacement = FindLeaf(node_ptr->left.get());
      // a subtle point: if the replacement node is a direct child of the node
      // being replaced, then the lowest node affected by the change will be the
      // replacement, and the replacement's parent will be gone soon. Otherwise,
      // the lowest affected node is the replacement's parent, which lost a
      // child but is still in the tree.
      to_rebalance = IsDirectChild(node_ptr, replacement) ? replacement
                                                          : replacement->parent;
      ReplaceDeletedNodeWithLeaf(node_ptr, replacement);
    }
    FixWeightsAndRebalance(to_rebalance);
    // node_ptr has already been destroyed by the unique_ptr operations above;
    // just clean up the map entry.
    element_to_node_.erase(element);
    return std::make_optional(removed_value);
  }

  ~WeightedSet() = default;

  size_t size() const { return element_to_node_.size(); }
  bool empty() const { return size() == 0; }
  double total_weight() const { return root_ ? root_->subtree_weight : 0.0; }
  bool Contains(const T& element) const {
    return element_to_node_.find(element) != element_to_node_.end();
  }

  // (Claude-written) visualization: prints ASCII tree to ostream (default:
  // cout)
  void DebugPrint(std::ostream& os = std::cout) const {
    if (!root_) {
      os << "(empty tree)" << std::endl;
      return;
    }
    DebugPrintNode(os, root_.get(), "", false, true);
  }

  // (Claude-written): return tree as string instead of printing
  std::string DebugString() const {
    std::ostringstream oss;
    DebugPrint(oss);
    return oss.str();
  }
};

}  // namespace cse498
