#pragma once
#include <cstddef>
#include <expected>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace cse498 {
/*
  Forgot to mention this in earlier commit
  But the boilerplate setup (constructors, Clear() and CopyTree(), assign and
  move operators, basic getters [namely size(), empty(), total_weight()], and
  basic member variables) was written by Claude Code with (so far moderate)
  modification from me.
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
    // Note on the implementation: the value itself is owned and stored in the unordered_map used
    // in WeightedSet--we need to have the values as keys in the unordered_map so we can use it for membership checking,
    // and once we've stored the value there, it would be a waste to duplicate it in the node.
    const T* value_ptr;
    double weight;          // this element's weight (nonnegative)
    double subtree_weight;  // total weight of subtree rooted at this node
    Node* left = nullptr; // left child
    Node* right = nullptr; // right child
    Node* parent;

    Node(const T* val_ptr, double w, Node* par = nullptr)
        : value_ptr(val_ptr),
          weight(w),
          subtree_weight(w),
          left(nullptr),
          right(nullptr),
          parent(par) {}
  };

  Node* root_;
  // TODO: replace this with custom random class once we have that
  std::random_device rd_{};
  std::mt19937 rng_{rd_()};
  // Keys are elements stored in the WeightedSet; values are the nodes in the tree representing that element.
  std::unordered_map<T, Node*> element_to_node_;
  // Deletes the whole tree starting at a given node.
  // Note: not sufficient to delete the WeightedSet; you also need to get rid of the 
  // unordered_map and the values stored within it.
  void Clear(Node* node) {
    if (!node) return;
    Clear(node->left);
    Clear(node->right);
    delete node;
  }

  Node* CopyTree(const Node* src, Node* parent) {
    if (!src) return nullptr;
    auto element = *(src->value_ptr);
    auto [iter, inserted] = element_to_node_.insert({element, nullptr});
    const T* element_ptr = &(iter->first);
    Node* node = new Node(element_ptr, src->weight, parent);
    iter->second = node;
    node->subtree_weight = src->subtree_weight;
    node->left = CopyTree(src->left, node);
    node->right = CopyTree(src->right, node);
    return node;
  }

  double LeftSubtreeWeight(Node* node) {
    return node->left ? node->left->subtree_weight : 0.0;
  }

  double RightSubtreeWeight(Node* node) {
    return node->right ? node->right->subtree_weight : 0.0;
  }
  // Ensures that a given node and all its ancestors have their total subtree weight recorded correctly.
  // Generally you pass in the lowest node that had its subtree change during an insertion or deletion: then
  // it and all of its ancestors may also have had their subtree weights change as a result.
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
  void DebugPrintNode(std::ostream& os, Node* node, const std::string& prefix,
                        bool is_left, bool is_root) const {
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
      DebugPrintNode(os, node->left, child_prefix, true, false);
      DebugPrintNode(os, node->right, child_prefix, false, false);
    }
  }

  bool IsDirectChild(Node* parent, Node* child) {
    return (parent->left == child) || (parent->right == child);
  }

  void ReplaceDeletedNodeWithChild(Node* to_delete, Node* replacement) {
    if (to_delete->parent == nullptr) {
      this->root_ = replacement;
    } else if (to_delete == to_delete->parent->left) {
      to_delete->parent->left = replacement;
    } else {
      to_delete->parent->right = replacement;
    }
    if (replacement != nullptr) {
      replacement->parent = to_delete->parent;
    }
  }
  // Searches for a child of the given node which is a leaf (such a child exists in any tree, as long as the given
  // node has at least one child).
  Node* FindLeaf(Node* current) {
    if (current->left == nullptr && current->right == nullptr) {
      return current;
    }
    if (current->left != nullptr) {
      return FindLeaf(current->left);
    } else {
      return FindLeaf(current->right);
    }
  }
  //we take a leaf node and swap it in for the node to be deleted.
  //the leaf's left, right, and parent should become the left, right, and parent of the deleted node.
  //then we need to rebalance starting at the previous parent of the leaf (if that isn't the deleted node)
  //or starting at the moved leaf (if the leaf was a child of the deleted node).

  void ReplaceDeletedNodeWithLeaf(Node* to_delete, Node* leaf) {
    if (to_delete->parent == nullptr) {
      this->root_ = leaf;
    } else if (to_delete == to_delete->parent->left) {
      to_delete->parent->left = leaf;
    } else {
      to_delete->parent->right = leaf;
    }
    if (leaf != nullptr) {
      if (leaf->parent->left == leaf) {
        leaf->parent->left = nullptr;
      } else {
        leaf->parent->right = nullptr;
      }
    }
    leaf->parent = to_delete->parent;
    if (leaf != to_delete->left) {
      leaf->left = to_delete->left;
    }
    if (leaf != to_delete->right) {
      leaf->right = to_delete->right;
    }
    if (leaf->left != nullptr) {
      leaf->left->parent = leaf;
    }
    if (leaf->right != nullptr) {
      leaf->right->parent = leaf;
    }
  }

 public:
  WeightedSet() : root_(nullptr) {}

  WeightedSet(const WeightedSet& other)
      : root_(CopyTree(other.root_, nullptr)) {}

  WeightedSet& operator=(const WeightedSet& other) {
    if (this != &other) {
      Clear(root_);
      element_to_node_.clear();
      root_ = CopyTree(other.root_, nullptr);
    }
    return *this;
  }

  WeightedSet& operator=(WeightedSet&& other) noexcept {
    if (this != &other) {
      Clear(root_);
      root_ = other.root_;
      other->Clear();

      other.root_ = nullptr;
    }
    return *this;
  }
  // Insert an element into the tree with a given weight. Returns whether the insertion is successful.
  // If the element is already there, we simply update its weight.
  // When inserting a new element, we (recursively) put it in the subtree which currently has the lightest weight, to keep the tree
  // weight-balanced (which should theoretically improve the speed of the GetRandomElement function).
  // The insertion fails if the weight is invalid (it must be positive) or if the function somehow fails
  // to find a place to insert the element.
  bool Insert(T element, double weight) {
    if (weight <= 0) {
      return false;
    }

    if (this->element_to_node_.find(element) != element_to_node_.end()) {
      auto node_ptr = this->element_to_node_.at(element);
      node_ptr->weight = weight;
      FixWeightsAndRebalance(node_ptr);
      return true;
    }
    // note: this code, to obtain a pointer to the T element stored in the map,
    // was modified from a snippet by Claude. It replaces an older version that created two copies of the element,
    // one stored in the map and one pointed to by the node's value_ptr
    auto [iter, inserted] = element_to_node_.insert({element, nullptr});
    const T* element_ptr = &(iter->first);
    Node* node_ptr = new Node(element_ptr, weight);
    iter->second = node_ptr;

    if (root_ == nullptr) {
      this->root_ = node_ptr;
      return true;
    }
    Node* current_node = this->root_;
    while (current_node != nullptr) {
      // if a node has less than 2 children, we can just insert it in one of the empty spaces.
      if (current_node->left == nullptr) {
        current_node->left = node_ptr;
        node_ptr->parent = current_node;
        FixWeightsAndRebalance(current_node);
        return true;
      } else if (current_node->right == nullptr) {
        current_node->right = node_ptr;
        node_ptr->parent = current_node;
        FixWeightsAndRebalance(current_node);
        return true;
      // if a node does have 2 children, we insert it in the lighter of the two subtrees.
      } else if (current_node->left->subtree_weight <
                 current_node->right->subtree_weight) {
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }
    return false;
  }

  std::optional<T> GetElementAt(double index) {
    Node* current_node = this->root_;
    double total = this->root_->subtree_weight;
    if (index < 0 || index > total) {
      return std::nullopt;
    }
    double lower_bound = 0;
    double upper_bound = this->root_->subtree_weight;
    while (current_node != nullptr) {
      double current_node_interval_lower = lower_bound + (current_node->left ? current_node->left->subtree_weight : 0);
      double current_node_interval_upper = current_node_interval_lower + current_node->weight;
      if (index < current_node_interval_lower) {
        upper_bound = current_node_interval_lower;
        current_node = current_node->left;
      } else if (index >= current_node_interval_upper && current_node_interval_upper != upper_bound) {
        lower_bound = current_node_interval_upper;
        current_node = current_node->right;
      } else {
        return std::make_optional(*(current_node->value_ptr));
      }
    }
    return std::nullopt;
  }

  std::optional<T> GetRandomElement() {
    if (this->root_ == nullptr) {
      return std::nullopt;
    }
    auto random_real =
        std::uniform_real_distribution<double>(0, this->root_->subtree_weight);
    return GetElementAt(random_real(rng_));
  }

  std::optional<T> Remove(const T& element) {
    if (this->element_to_node_.find(element) == element_to_node_.end()) {
      return std::nullopt;
    }
    Node* node_ptr = this->element_to_node_.at(element);
    T removed_value = *(node_ptr->value_ptr);

    Node* to_rebalance = node_ptr->parent;
    if (node_ptr->left == nullptr && node_ptr->right == nullptr) {
      ReplaceDeletedNodeWithChild(node_ptr, nullptr);
    } else if (node_ptr->left == nullptr) {
      to_rebalance = node_ptr->right;
      ReplaceDeletedNodeWithChild(node_ptr, node_ptr->right);
    } else if (node_ptr->right == nullptr) {
      to_rebalance = node_ptr->left;
      ReplaceDeletedNodeWithChild(node_ptr, node_ptr->left);
    } else {
      Node* replacement = FindLeaf(node_ptr->left);
      // a subtle point: if the replacement node is a direct child of the node being replaced,
      // then the lowest node affected by the change will be the replacement, and the replacement's parent will be gone soon. 
      // Otherwise, the lowest affected node is the replacement's parent, which lost a child but is still in the tree.
      to_rebalance = IsDirectChild(node_ptr, replacement) ? replacement : replacement->parent;
      ReplaceDeletedNodeWithLeaf(node_ptr, replacement);
    }
    FixWeightsAndRebalance(to_rebalance);
    delete node_ptr;
    element_to_node_.erase(element);
    return std::make_optional(removed_value);
  }
  ~WeightedSet() {
    Clear(root_);
    element_to_node_.clear();
  }

  size_t size() const { return element_to_node_.size(); }
  bool empty() const { return size() == 0; }
  double total_weight() const { return root_ ? root_->subtree_weight : 0.0; }
  bool Contains(const T& element) {
    return element_to_node_.find(element) != element_to_node_.end();
  }

  // (Claude-written) visualization: prints ASCII tree to ostream (default:
  // cout)
  void DebugPrint(std::ostream& os = std::cout) const {
    if (!root_) {
      os << "(empty tree)" << std::endl;
      return;
    }
    DebugPrintNode(os, root_, "", false, true);
  }

  // (Claude-written): return tree as string instead of printing
  std::string DebugString() const {
    std::ostringstream oss;
    DebugPrint(oss);
    return oss.str();
  }
};

}  // namespace cse498
