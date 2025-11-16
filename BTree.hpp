#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <optional>
#include <type_traits>

// ----------------------------------------------
// BTree: Binary Search Tree
// ----------------------------------------------

// Unbalanced binary search tree (BST).
template<typename K, typename V>
class BTree final {
    struct Node;

public:
    enum class Result: int {
        Inserted,
        Updated,
        Failed,
    };

    // R-value reference:
    auto insert(K&& key, V&& value) -> Result {
        const bool allow_update = false;
        return try_insert_key_val(nullptr, m_root, std::move(key), std::move(value), allow_update);
    }

    auto insert_or_update(K&& key, V&& value) -> Result {
        const bool allow_update = true;
        return try_insert_key_val(nullptr, m_root, std::move(key), std::move(value), allow_update);
    }

    // Const-ref key, R-value ref value:
    auto insert(const K& key, V&& value) -> Result {
        const bool allow_update = false;
        return try_insert_key_val(nullptr, m_root, key, std::move(value), allow_update);
    }

    auto insert_or_update(const K& key, V&& value) -> Result {
        const bool allow_update = true;
        return try_insert_key_val(nullptr, m_root, key, std::move(value), allow_update);
    }

    // Const-reference (copy):
    auto insert(const K& key, const V& value) -> Result {
        const bool allow_update = false;
        return try_insert_key_val(nullptr, m_root, key, value, allow_update);
    }

    auto insert_or_update(const K& key, const V& value) -> Result {
        const bool allow_update = true;
        return try_insert_key_val(nullptr, m_root, key, value, allow_update);
    }

    // Remove by key:
    auto remove(const K& key) -> std::optional<V> {
        return try_remove_key(m_root, key);
    }

    using ValueConstRef = std::reference_wrapper<const V>;

    // Find by key:
    auto find(const K& key) const -> std::optional<ValueConstRef> {
        return try_find_key(m_root, key);
    }

    auto clear() -> void {
        m_root = nullptr;
        m_size = 0;
    }

    auto size() const -> std::size_t {
        return m_size;
    }

    auto is_empty() const -> bool {
        return m_root == nullptr;
    }

    auto validate() const -> void {
        if (is_empty()) {
            assert(size() == 0);
        } else {
            assert(size() != 0);
            validate_parents(m_root.get(), nullptr);
        }
    }

    // Inorder (Left, Root, Right), ascending key order.
    template<typename VisitorFn>
    auto visit_inorder(const VisitorFn& visitor) const -> void {
        visit_inorder(m_root, visitor);
    }

    // Preorder (Root, Left, Right).
    template<typename VisitorFn>
    auto visit_preorder(const VisitorFn& visitor) const -> void {
        visit_preorder(m_root, visitor);
    }

    // Postorder (Left, Right, Root).
    template<typename VisitorFn>
    auto visit_postorder(const VisitorFn& visitor) const -> void {
        visit_postorder(m_root, visitor);
    }

    // Non-recursive iteration:
    template<typename BTreeType, typename NodeType, typename ValueType>
    class InorderIter final {
    public:
        using value_type        = std::pair<const K&, ValueType&>;
        using reference         = value_type&;
        using pointer           = value_type*;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        auto operator*() const -> value_type {
            assert(m_node != nullptr);
            return { m_node->key, m_node->value };
        }

        auto operator->() const -> value_type {
            assert(m_node != nullptr);
            return { m_node->key, m_node->value };
        }

        auto operator++() -> InorderIter& { // ++iter
            m_node = BTree::next_node(m_node);
            return *this;
        }

        auto operator++(int) -> InorderIter { // iter++
            auto temp = *this;
            ++(*this);
            return temp;
        }

        auto operator--() -> InorderIter& { // --iter
            if (m_node == nullptr) {
                // If we started from tree.end()
                m_node = BTree::find_max(m_tree->m_root.get());
            } else {
                m_node = BTree::prev_node(m_node);
            }
            return *this;
        }

        auto operator--(int) -> InorderIter { // iter--
            auto temp = *this;
            --(*this);
            return temp;
        }

        auto operator==(const InorderIter& rhs) const -> bool {
            return m_tree == rhs.m_tree && m_node == rhs.m_node;
        }

        auto operator!=(const InorderIter& rhs) const -> bool {
            return !(*this == rhs);
        }

        // iterator -> const_iterator conversion
        template<typename BT = BTreeType, typename ND = NodeType, typename VL = ValueType>
        requires (!std::is_const_v<ND>)
        operator InorderIter<const BT, const ND, const VL>() const {
            return InorderIter<const BT, const ND, const VL>(m_tree, m_node);
        }

    private:
        friend class BTree;

        InorderIter(BTreeType* const tree, NodeType* start)
            : m_tree{tree}
            , m_node{start}
        {}
        
        BTreeType* const m_tree;
        NodeType* m_node; // current node.
    };

    using iterator = InorderIter<BTree, Node, V>;
    using const_iterator = InorderIter<const BTree, const Node, const V>;

    auto begin() -> iterator {
        return iterator{this, find_min(m_root.get())};
    }

    auto end() -> iterator {
        return iterator{this, nullptr};
    }

    auto begin() const -> const_iterator {
        return const_iterator{this, find_min(m_root.get())};
    }

    auto end() const -> const_iterator {
        return const_iterator{this, nullptr};
    }

private:
    struct Node {
        K key;
        V value;
        Node* parent; // null for root.
        std::unique_ptr<Node> left{nullptr};
        std::unique_ptr<Node> right{nullptr};

        template<typename Key, typename Val>
        Node(Node* const parent, Key&& key, Val&& value)
            : key{std::forward<Key>(key)}
            , value{std::forward<Val>(value)}
            , parent{parent}
        {}
    };

    template<typename Key, typename Val>
    auto try_insert_key_val(Node* const parent,
                            std::unique_ptr<Node>& node,
                            Key&& key,
                            Val&& value,
                            const bool allow_update) -> Result {
        if (node == nullptr) {
            // End of subtree; insert here.
            node = std::make_unique<Node>(parent,
                                          std::forward<Key>(key),
                                          std::forward<Val>(value));
            ++m_size;
            return Result::Inserted;
        }

        // Insert left:
        if (key < node->key) {
            return try_insert_key_val(node.get(),
                                      node->left,
                                      std::forward<Key>(key),
                                      std::forward<Val>(value),
                                      allow_update);
        }

        // Insert right:
        if (key > node->key) {
            return try_insert_key_val(node.get(),
                                      node->right,
                                      std::forward<Key>(key),
                                      std::forward<Val>(value),
                                      allow_update);
        }

        // Key already exists.
        assert(key == node->key);
        assert(parent == node->parent);

        if (allow_update) {
            node->value = value;
            return Result::Updated;
        }

        return Result::Failed;
    }

    auto try_remove_key(std::unique_ptr<Node>& node,
                        const K& key) -> std::optional<V> {
        if (node == nullptr) {
            // End of subtree; not found.
            return std::nullopt;
        }

        // Search left:
        if (key < node->key) {
            return try_remove_key(node->left, key);
        }

        // Search right:
        if (key > node->key) {
            return try_remove_key(node->right, key);
        }

        // Found the node to remove.
        assert(key == node->key);

        // Handle 3 cases:
        //  1. Leaf node: no children - just remove it.
        //  2. One child: replace the node with its child.
        //  3. Two children: find inorder successor (smallest value in right subtree),
        //                   copy it into the node, and delete that successor.

        // Case 1 & 2: node has 0 or 1 child.
        if (node->left == nullptr || node->right == nullptr) {
            std::optional<V> removed_value{std::move(node->value)};

            // Replace current node by whichever child is not null (or nullptr for leaf).
            if (node->left != nullptr) {
                assert(node->left->parent == node.get());
                node->left->parent = node->parent;
                node = std::move(node->left);
            } else if (node->right != nullptr) {
                assert(node->right->parent == node.get());
                node->right->parent = node->parent;
                node = std::move(node->right);
            } else {
                node = nullptr;
            }

            --m_size;
            return removed_value;
        }

        // Case 3: node has two children.
        auto successor = find_min(node->right.get());
        assert(successor != nullptr);

        // Save removed value before overwriting current node’s value.
        std::optional<V> removed_value{std::move(node->value)};

        // Copy successor’s key/value into this node.
        node->key   = successor->key;
        node->value = std::move(successor->value);

        // Remove the successor node recursively.
        [[maybe_unused]] const auto result = try_remove_key(node->right, successor->key);
        assert(result.has_value());

        // Don't decrement m_size again here (the recursive call already did).
        return removed_value;
    }

    static auto find_min(Node* node) -> Node* {
        if (node == nullptr) {
            return nullptr;
        }

        while (node->left != nullptr) {
            assert(node->left->parent == node);
            node = node->left.get();
        }

        return node;
    }

    static auto find_max(Node* node) -> Node* {
        if (node == nullptr) {
            return nullptr;
        }

        while (node->right != nullptr) {
            assert(node->right->parent == node);
            node = node->right.get();
        }

        return node;
    }

    static auto next_node(Node* node) -> Node* {
        if (node == nullptr) {
            return nullptr;
        }

        if (node->right != nullptr) {
            return find_min(node->right.get());
        }

        while (node->parent != nullptr && node == node->parent->right.get()) {
            node = node->parent;
        }

        return node->parent;
    }

    static auto prev_node(Node* node) -> Node* {
        if (node == nullptr) {
            return nullptr;
        }

        if (node->left != nullptr) {
            return find_max(node->left.get());
        }

        while (node->parent != nullptr && node == node->parent->left.get()) {
            node = node->parent;
        }

        return node->parent;
    }

    static auto try_find_key(const std::unique_ptr<Node>& node,
                             const K& key) -> std::optional<ValueConstRef> {
        if (node == nullptr) {
            // End of subtree; not found.
            return std::nullopt;
        }

        // Search left:
        if (key < node->key) {
            return try_find_key(node->left, key);
        }

        // Search right:
        if (key > node->key) {
            return try_find_key(node->right, key);
        }

        // Found key.
        assert(key == node->key);
        return std::optional{std::ref(node->value)};
    }

    static auto validate_parents(const Node* const node, const Node* const expected_parent) -> void {
        if (node == nullptr) {
            return;
        }
        assert(node->parent == expected_parent);
        validate_parents(node->left.get(), node);
        validate_parents(node->right.get(), node);
    }

    // Recursive tree traversal:
    template<typename VisitorFn>
    static auto visit_inorder(const std::unique_ptr<Node>& node,
                              const VisitorFn& visitor) -> bool {
        if (node == nullptr) {
            return true;
        }

        if (!visit_inorder(node->left, visitor)) {
            return false;
        }

        if (!visitor(node->key, node->value)) {
            return false;
        }

        return visit_inorder(node->right, visitor);
    }

    template<typename VisitorFn>
    static auto visit_preorder(const std::unique_ptr<Node>& node,
                               const VisitorFn& visitor) -> bool {
        if (node == nullptr) {
            return true;
        }

        if (!visitor(node->key, node->value)) {
            return false;
        }

        if (!visit_preorder(node->left, visitor)) {
            return false;
        }

        return visit_preorder(node->right, visitor);
    }

    template<typename VisitorFn>
    static auto visit_postorder(const std::unique_ptr<Node>& node,
                                const VisitorFn& visitor) -> bool {
        if (node == nullptr) {
            return true;
        }

        if (!visit_postorder(node->left, visitor)) {
            return false;
        }

        if (!visit_postorder(node->right, visitor)) {
            return false;
        }

        return visitor(node->key, node->value);
    }

private:
    std::unique_ptr<Node> m_root{nullptr};
    std::size_t m_size{0};
};
