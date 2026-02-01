#pragma once
#include <cstddef>
#include <utility>

namespace cse498 {

template <typename T>
class WeightedSet {
private:
    struct Node {
        T value;
        double weight;          // this element's weight (nonnegative)
        double subtree_weight;  // total weight of subtree rooted at this node
        Node* left;
        Node* right;
        Node* parent;

        Node(const T& val, double w, Node* par = nullptr)
            : value(val), weight(w), subtree_weight(w),
              left(nullptr), right(nullptr), parent(par) {}
    };

    Node* root_;
    size_t size_;

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

    ~WeightedSet() { clear(root_); }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    double total_weight() const { return root_ ? root_->subtree_weight : 0.0; }
};

}
