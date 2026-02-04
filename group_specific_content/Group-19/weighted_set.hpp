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
  But the boilerplate setup (constructors, clear() and copy_tree(), assign and
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
  struct Node {
    T* value_ptr;
    double weight;          // this element's weight (nonnegative)
    double subtree_weight;  // total weight of subtree rooted at this node
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent;

    Node(T* val_ptr, double w, Node* par = nullptr)
        : value_ptr(val_ptr),
          weight(w),
          subtree_weight(w),
          left(nullptr),
          right(nullptr),
          parent(par) {}
  };

  Node* root_;
  size_t size_;
  // TODO: replace this with custom random class once we have that
  std::random_device rd{};
  std::mt19937 rng{rd()};

  std::unordered_map<T, Node*> element_to_node_;

  void clear(Node* node) {
    if (!node) return;
    clear(node->left);
    clear(node->right);
    delete node;
  }

  Node* copy_tree(const Node* src, Node* parent) {
    if (!src) return nullptr;
    Node* node = new Node(src->value, src->weight, parent);
    node->subtree_weight = src->subtree_weight;
    node->left = copy_tree(src->left, node);
    node->right = copy_tree(src->right, node);
    return node;
  }

  void fix_weights_and_rebalance(Node* node_ptr) {
    while (node_ptr != nullptr) {
      // TODO: rotate if doing so would make the tree better weight-balanced
      node_ptr->subtree_weight =
          node_ptr->weight +
          (node_ptr->left != nullptr ? node_ptr->left->subtree_weight : 0) +
          (node_ptr->right != nullptr ? node_ptr->right->subtree_weight : 0);
      node_ptr = node_ptr->parent;
    }
  }

  // (Claude-written) helper for debug_print: recursively builds tree visualization
  void debug_print_node(std::ostream& os, Node* node, const std::string& prefix,
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

    std::string child_prefix = prefix + (is_root ? "" : (is_left ? "│   " : "    "));

    if (node->left || node->right) {
      debug_print_node(os, node->left, child_prefix, true, false);
      debug_print_node(os, node->right, child_prefix, false, false);
    }
  }

  void replace_deleted_node(Node* to_delete, Node* replacement) {
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

 public:
  WeightedSet() : root_(nullptr), size_(0) {}

  WeightedSet(const WeightedSet& other)
      : root_(copy_tree(other.root_, nullptr)), size_(other.size_) {}

  WeightedSet& operator=(const WeightedSet& other) {
    if (this != &other) {
      clear(root_);
      root_ = copy_tree(other.root_, nullptr);
      size_ = other.size_;
    }
    return *this;
  }

  WeightedSet& operator=(WeightedSet&& other) noexcept {
    if (this != &other) {
      clear(root_);
      root_ = other.root_;
      size_ = other.size_;
      other.root_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  bool insert(T element, double weight) {
    if (weight <= 0) {
      return false;
    }
    if (root_ == nullptr) {
      T* element_ptr = new T{element};
      Node* node_ptr = new Node(element_ptr, weight);
      this->element_to_node_[*element_ptr] = node_ptr;
      this->root_ = node_ptr;
      return true;
    }

    if (this->element_to_node_.find(element) != element_to_node_.end()) {
      auto node_ptr = this->element_to_node_.at(element);
      node_ptr->weight = weight;
      fix_weights_and_rebalance(node_ptr);
    }
    T* element_ptr = new T{element};
    Node* node_ptr = new Node(element_ptr, weight);
    this->element_to_node_[*element_ptr] = node_ptr;
    Node* current_node = this->root_;
    while (current_node != nullptr) {
      if (current_node->left == nullptr) {
        current_node->left = node_ptr;
        node_ptr->parent = current_node;
        fix_weights_and_rebalance(current_node);
        return true;
      } else if (current_node->right == nullptr) {
        current_node->right = node_ptr;
        node_ptr->parent = current_node;
        fix_weights_and_rebalance(current_node);
        return true;
      } else if (current_node->left->subtree_weight <
                 current_node->right->subtree_weight) {
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }
    return false;
  }

  std::optional<T> getElementAt(double index) {
    Node* current_node = this->root_;
    double total = this->root_->subtree_weight;
    if (index < 0 || index > total) {
      return std::nullopt;
    }
    double lower_bound;
    double upper_bound;
    while (current_node != nullptr) {
      lower_bound = current_node->left ? current_node->left->subtree_weight : 0;
      upper_bound = lower_bound + current_node->weight;
      if (index < lower_bound) {
        current_node = current_node->left;
      } else if (index >= upper_bound && upper_bound != total) {
        current_node = current_node->right;
      } else {
        return std::make_optional(*(current_node->value_ptr));
      }
    }
    return std::nullopt;
  }

  std::optional<T> getRandomElement() {
    if (this->root_ == nullptr) {
      return std::nullopt;
    }
    auto random_real =
        std::uniform_real_distribution<double>(0, this->root_->subtree_weight);
    return getElementAt(random_real(rng));
  }

  std::optional<T> remove(const T& element) {
    if (this->element_to_node_.find(element) == element_to_node_.end()) {
      return std::nullopt;
    }
    Node* node_ptr = this->element_to_node_.at(element);
    T removed_value = *(node_ptr->value_ptr);

    if (node_ptr->left == nullptr && node_ptr->right == nullptr) {
      replace_deleted_node(node_ptr, nullptr);
    } else if (node_ptr->left == nullptr) {
      replace_deleted_node(node_ptr, node_ptr->right);
    } else if (node_ptr->right == nullptr) {
      replace_deleted_node(node_ptr, node_ptr->left);
    } else {
      // TODO: handle case where both children exist
    }
    fix_weights_and_rebalance(node_ptr->parent);
    delete node_ptr;
    element_to_node_.erase(element);
    return std::make_optional(removed_value);
  }
  ~WeightedSet() { 
    clear(root_);
    element_to_node_.clear();
   }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  double total_weight() const { return root_ ? root_->subtree_weight : 0.0; }
  bool contains(const T& element) {
    return element_to_node_.find(element) != element_to_node_.end();
  }

  // (Claude-written) visualization: prints ASCII tree to ostream (default: cout)
  void debug_print(std::ostream& os = std::cout) const {
    if (!root_) {
      os << "(empty tree)" << std::endl;
      return;
    }
    debug_print_node(os, root_, "", false, true);
  }

  // (Claude-written): return tree as string instead of printing
  std::string debug_string() const {
    std::ostringstream oss;
    debug_print(oss);
    return oss.str();
  }
};

}  // namespace cse498
