#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <optional>
#include <type_traits>

// ----------------------------------------------
// RBTree: Red-Black Balanced Tree
// ----------------------------------------------

// Heavily based on EternallyConfuzzled's tutorial on Red-Black trees here:
//  http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
// Or from the web archive:
//  https://web.archive.org/web/20100430180019/http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx

template<typename K, typename V>
class RBTree final {
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
        const auto result = try_insert_key_val(nullptr, m_root, std::move(key), std::move(value), allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    auto insert_or_update(K&& key, V&& value) -> Result {
        const bool allow_update = true;
        const auto result = try_insert_key_val(nullptr, m_root, std::move(key), std::move(value), allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    // Const-ref key, R-value ref value:
    auto insert(const K& key, V&& value) -> Result {
        const bool allow_update = false;
        const auto result = try_insert_key_val(nullptr, m_root, key, std::move(value), allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    auto insert_or_update(const K& key, V&& value) -> Result {
        const bool allow_update = true;
        const auto result = try_insert_key_val(nullptr, m_root, key, std::move(value), allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    // Const-reference (copy):
    auto insert(const K& key, const V& value) -> Result {
        const bool allow_update = false;
        const auto result = try_insert_key_val(nullptr, m_root, key, value, allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    auto insert_or_update(const K& key, const V& value) -> Result {
        const bool allow_update = true;
        const auto result = try_insert_key_val(nullptr, m_root, key, value, allow_update);
        if (result == Result::Inserted) {
            m_root->color = Color::Black;
        }
        return result;
    }

    auto remove(const K& key) -> std::optional<V> {
        bool is_done = false;
        const auto result = try_remove_key(m_root, is_done, key);
        if (result.has_value() && !is_empty()) {
            m_root->color = Color::Black;
        }
        return result;
    }

    using ValueConstRef = std::reference_wrapper<const V>;

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

            // Check RBTree invariants:
            const auto [is_valid, height] = validate_red_black_tree(m_root);
            if (!is_valid || height == 0) {
                assert(false && "Invalid Red-Black Tree");
            }
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
    template<typename RBTreeType, typename NodeType, typename ValueType>
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
            m_node = RBTree::next_node(m_node);
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
                m_node = RBTree::find_max(m_tree->m_root.get());
            } else {
                m_node = RBTree::prev_node(m_node);
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
        template<typename RBT = RBTreeType, typename ND = NodeType, typename VL = ValueType>
        requires (!std::is_const_v<ND>)
        operator InorderIter<const RBT, const ND, const VL>() const {
            return InorderIter<const RBT, const ND, const VL>(m_tree, m_node);
        }

    private:
        friend class RBTree;

        InorderIter(RBTreeType* const tree, NodeType* start)
            : m_tree{tree}
            , m_node{start}
        {}
        
        RBTreeType* const m_tree;
        NodeType* m_node; // current node.
    };

    using iterator = InorderIter<RBTree, Node, V>;
    using const_iterator = InorderIter<const RBTree, const Node, const V>;

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
    using Direction = std::size_t;
    static constexpr Direction DIR_LEFT  = 0;
    static constexpr Direction DIR_RIGHT = 1;

    enum class Color: std::uint8_t {
        Red,
        Black
    };

    struct Node {
        K key;
        V value;
        Node* parent; // null for root.
        std::unique_ptr<Node> left{nullptr};
        std::unique_ptr<Node> right{nullptr};
        Color color;

        template<typename Key, typename Val>
        Node(Node* const parent, const Color color, Key&& key, Val&& value)
            : key{std::forward<Key>(key)}
            , value{std::forward<Val>(value)}
            , parent{parent}
            , color{color}
        {}

        auto is_red()   const -> bool { return color == Color::Red; }
        auto is_black() const -> bool { return color == Color::Black; }

        auto child(const Direction dir) -> std::unique_ptr<Node>& {
            switch (dir) {
            case DIR_LEFT  : return left;
            case DIR_RIGHT : return right;
            default: assert(false && "Invalid child index");
            }
        }
    };

    static auto single_rotation(std::unique_ptr<Node>& root, const Direction dir) -> void {
        //  dir = direction of rotation (DIR_LEFT or DIR_RIGHT)
        // !dir = opposite direction.
        const Direction odir = !dir;

        // Save raw pointer to old root BEFORE moving anything.
        Node* const old_root = root.get();
        Node* const old_parent = old_root->parent;

        // Promote: save = root->child(odir)
        auto save = std::move(old_root->child(odir));

        // Recolor:
        old_root->color = Color::Red;
        save->color = Color::Black;

        // Step 1: The subtree that moves under old_root.
        old_root->child(odir) = std::move(save->child(dir));
        if (old_root->child(odir) != nullptr) {
            old_root->child(odir)->parent = old_root;
        }

        // Step 2: old_root becomes child(dir) of save.
        save->child(dir) = std::move(root);
        save->child(dir)->parent = save.get();

        // Step 3: save becomes the new root of this subtree.
        save->parent = old_parent;
        root = std::move(save);
    }

    static auto double_rotation(std::unique_ptr<Node>& root, const Direction dir) -> void {
        single_rotation(root->child(!dir), !dir);
        single_rotation(root, dir);
    }

    static auto insert_rebalance(std::unique_ptr<Node>& root, const Direction dir) -> void {
        const auto& child = root->child(dir);
        const auto& opposite_child = root->child(!dir);

        if (child != nullptr && child->is_red()) {
            if (opposite_child != nullptr && opposite_child->is_red()) {
                root->color = Color::Red;
                root->left->color  = Color::Black;
                root->right->color = Color::Black;
            } else {
                const auto& grandchild = child->child(dir);
                const auto& opposite_grandchild = child->child(!dir);

                if (grandchild != nullptr && grandchild->is_red()) {
                    single_rotation(root, !dir);
                } else if (opposite_grandchild != nullptr && opposite_grandchild->is_red()) {
                    double_rotation(root, !dir);
                } else {
                    // No rebalancing needed.
                }
            }
        }
    }

    static auto remove_rebalance(std::unique_ptr<Node>& root, const Direction dir) -> bool {
        Node* s = root->child(!dir).get();

        // Case 1: Handle red sibling
        if (s != nullptr && s->is_red()) {
            single_rotation(root, dir);
            s = root->child(!dir).get();
        }

        bool is_done = false;

        if (s != nullptr) {
            const bool left_red  = (s->left  && s->left->is_red());
            const bool right_red = (s->right && s->right->is_red());

            // Case 2: No red children
            if (!left_red && !right_red) {
                if (root->is_red()) {
                    is_done = true;
                }
                root->color = Color::Black;
                s->color = Color::Red;
            }
            else {
                const Color saved_color = root->color;

                // Case 3: One or both red children
                if (s->child(!dir) && s->child(!dir)->is_red()) {
                    single_rotation(root, dir);
                } else {
                    double_rotation(root, dir);
                }

                root->color = saved_color;
                root->left->color  = Color::Black;
                root->right->color = Color::Black;

                is_done = true;
            }
        }

        return is_done;
    }

    // Returns a pair of bool indicating if the tree is valid and if so the black height of the entire tree.
    static auto validate_red_black_tree(const std::unique_ptr<Node>& root) -> std::pair<bool, std::size_t> {
        if (root == nullptr) {
            return {true, 1};
        }

        const auto& left_child  = root->left;
        const auto& right_child = root->right;

        // Consecutive red links?
        if (root->is_red()) {
            const bool left_red  = left_child  != nullptr && left_child->is_red();
            const bool right_red = right_child != nullptr && right_child->is_red();
            if (left_red || right_red) {
                assert(false && "Red violation");
                return {false, 0};
            }
        }

        const auto [left_valid,  left_height]  = validate_red_black_tree(left_child);
        const auto [right_valid, right_height] = validate_red_black_tree(right_child);

        // Invalid binary search tree?
        if ((left_child  != nullptr && left_child->key  >= root->key) ||
            (right_child != nullptr && right_child->key <= root->key))
        {
            assert(false && "Binary tree violation");
            return {false, 0};
        }

        // Black height mismatch?
        if (left_height != 0 && right_height != 0 && left_height != right_height) {
            assert(false && "Black violation");
            return {false, 0};
        }

        // Only count black nodes.
        if (left_height != 0 && right_height != 0) {
            return {true, root->is_red() ? left_height : left_height + 1};
        }

        return {false, 0};
    }

    static auto validate_parents(const Node* const node, const Node* const expected_parent) -> void {
        if (node == nullptr) {
            return;
        }
        assert(node->parent == expected_parent);
        validate_parents(node->left.get(), node);
        validate_parents(node->right.get(), node);
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

    template<typename Key, typename Val>
    auto try_insert_key_val(Node* const parent,
                            std::unique_ptr<Node>& node,
                            Key&& key,
                            Val&& value,
                            const bool allow_update) -> Result {
        if (node == nullptr) {
            // End of subtree; insert here.
            node = std::make_unique<Node>(parent,
                                          Color::Red,
                                          std::forward<Key>(key),
                                          std::forward<Val>(value));
            ++m_size;
            return Result::Inserted;
        }

        // Insert left:
        if (key < node->key) {
            const auto result = try_insert_key_val(node.get(),
                                                   node->left,
                                                   std::forward<Key>(key),
                                                   std::forward<Val>(value),
                                                   allow_update);
            if (result == Result::Inserted) {
                insert_rebalance(node, DIR_LEFT);
            }
            return result;
        }

        // Insert right:
        if (key > node->key) {
            const auto result = try_insert_key_val(node.get(),
                                                   node->right,
                                                   std::forward<Key>(key),
                                                   std::forward<Val>(value),
                                                   allow_update);
            if (result == Result::Inserted) {
                insert_rebalance(node, DIR_RIGHT);
            }
            return result;
        }

        // Key already exists.
        assert(key == node->key);
        assert(parent == node->parent);

        if (allow_update) {
            node->value = value;
            // Value updated, tree structure unchanged.
            return Result::Updated;
        }

        // Tree structure unchanged.
        return Result::Failed;
    }

    auto try_remove_key(std::unique_ptr<Node>& root,
                        bool& is_done,
                        const K& key) -> std::optional<V> {
        if (root == nullptr) {
            // End of subtree; not found.
            is_done = true;
            return std::nullopt;
        }

        const K* key_ptr = &key;
        std::optional<V> removed_value{};

        if (root->key == *key_ptr) {
            if (root->left == nullptr || root->right == nullptr) {
                auto& child = root->child(root->left == nullptr);

                if (root->is_red()) {
                    is_done = true;
                } else if (child != nullptr && child->is_red()) {
                    child->color = Color::Black;
                    is_done = true;
                }

                removed_value = std::move(root->value);

                if (child != nullptr) {
                    assert(child->parent == root.get());
                    child->parent = root->parent;
                }
                root = std::move(child);
                --m_size;

                return removed_value;
            } else {
                auto successor = find_max(root->left.get());
                assert(successor != nullptr);

                removed_value = std::move(root->value);

                root->key   = successor->key;
                root->value = std::move(successor->value);

                key_ptr = &successor->key;
            }
        }

        const Direction dir = root->key < *key_ptr;
        const auto result = try_remove_key(root->child(dir), is_done, *key_ptr);

        if (!is_done) {
            is_done = remove_rebalance(root, dir);
        }

        return removed_value.has_value() ? removed_value : result;
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
