#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <optional>
#include <type_traits>

// ----------------------------------------------
// DList: Doubly-Linked List
// ----------------------------------------------

template<typename T>
class DList final {
private:
    struct Node {
        T value;
        Node* prev;
        std::unique_ptr<Node> next;

        template<typename... Args>
        Node(Node* prev, std::unique_ptr<Node>&& next, Args&&... args)
            : value{std::forward<Args>(args)...}
            , prev{prev}
            , next{std::move(next)}
        {}
    };

public:
    template<typename... Args>
    auto emplace_front(Args&&... args) -> void {
        m_head = std::make_unique<Node>(nullptr, std::move(m_head), std::forward<Args>(args)...);

        // More nodes following the list head?
        if (m_head->next != nullptr) {
            m_head->next->prev = m_head.get();
        } else {
            // Single node.
            assert(size() == 0);
            m_tail = m_head.get();
        }

        ++m_size;
    }

    template<typename U>
    auto push_front(U&& value) -> void
        requires std::constructible_from<T, U>
    {
        emplace_front(std::forward<U>(value));
    }

    auto pop_front() -> std::optional<T> {
        if (is_empty()) {
            return std::nullopt;
        }

        auto prev_head = std::move(m_head);
        m_head = std::move(prev_head->next);

        // Did we pop the last node?
        if (m_head != nullptr) {
            m_head->prev = nullptr;
        } else {
            // List cleared.
            assert(size() == 1);
            m_tail = nullptr;
        }

        --m_size;

        return std::optional{std::move(prev_head->value)};
    }

    template<typename... Args>
    auto emplace_back(Args&&... args) -> void {
        // Empty list? Same as emplace_front.
        if (is_empty()) {
            assert(m_tail == nullptr && size() == 0);
            emplace_front(std::forward<Args>(args)...);
        } else {
            assert(m_tail != nullptr);
            m_tail->next = std::make_unique<Node>(m_tail, nullptr, std::forward<Args>(args)...);
            m_tail = m_tail->next.get();
            ++m_size;
        }
    }

    template<typename U>
    auto push_back(U&& value) -> void
        requires std::constructible_from<T, U>
    {
        emplace_back(std::forward<U>(value));
    }

    auto pop_back() -> std::optional<T> {
        if (is_empty()) {
            return std::nullopt;
        }

        assert(m_tail != nullptr);
        auto value = std::move(m_tail->value);
        auto new_tail = m_tail->prev;

        if (new_tail != nullptr) {
            assert(new_tail->next == m_tail);
            new_tail->next = nullptr;
            m_tail = new_tail;
        } else {
            assert(m_head == m_tail && size() == 1);
            // Cleared the list.
            m_head = nullptr;
            m_tail = nullptr;
        }

        --m_size;

        return std::optional{std::move(value)};
    }

    auto clear() -> void {
        m_head = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    auto size() const -> std::size_t {
        return m_size;
    }

    auto is_empty() const -> bool {
        return m_head == nullptr;
    }

    auto validate() const -> void {
        if (is_empty()) {
            assert(size() == 0);
            assert(m_tail == nullptr);
        } else {
            assert(size() != 0);
            assert(m_tail != nullptr);

            const Node* prev_node = nullptr;
            for (const Node* node = m_head.get(); node != nullptr;) {
                assert(node->prev == prev_node);
                prev_node = node;
                node = node->next.get();
            }

            assert(prev_node == m_tail);
        }
    }

    auto front() -> T& {
        assert(!is_empty());
        return m_head->value;
    }

    auto front() const -> const T& {
        assert(!is_empty());
        return m_head->value;
    }

    auto back() -> T& {
        assert(!is_empty());
        return m_tail->value;
    }

    auto back() const -> const T& {
        assert(!is_empty());
        return m_tail->value;
    }

    template<typename NodeType, typename ValueType>
    class ForwardIter final {
    public:
        using value_type        = ValueType;
        using reference         = ValueType&;
        using pointer           = ValueType*;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        auto operator*() const -> reference {
            assert(m_current != nullptr);
            return m_current->value;
        }

        auto operator->() const -> pointer {
            assert(m_current != nullptr);
            return &m_current->value;
        }

        auto operator++() -> ForwardIter& { // ++iter
            assert(m_current != nullptr);
            m_current = m_current->next.get();
            return *this;
        }

        auto operator++(int) -> ForwardIter { // iter++
            auto temp = *this;
            ++(*this);
            return temp;
        }

        auto operator==(const ForwardIter& rhs) const -> bool {
            return m_current == rhs.m_current;
        }

        auto operator!=(const ForwardIter& rhs) const -> bool {
            return !(*this == rhs);
        }

        // iterator -> const_iterator conversion
        template<typename N = NodeType, typename V = ValueType>
        requires (!std::is_const_v<N>)
        operator ForwardIter<const N, const V>() const {
            return ForwardIter<const N, const V>(m_current);
        }

    private:
        friend class DList;

        explicit ForwardIter(NodeType* current)
            : m_current{current}
        {}

        NodeType* m_current;
    };

    using iterator = ForwardIter<Node, T>;
    using const_iterator = ForwardIter<const Node, const T>;

    auto begin() -> iterator {
        return iterator{m_head.get()};
    }

    auto end() -> iterator {
        return iterator{nullptr};
    }

    auto begin() const -> const_iterator {
        return const_iterator{m_head.get()};
    }

    auto end() const -> const_iterator {
        return const_iterator{nullptr};
    }

    template<typename NodeType, typename ValueType>
    class ReverseIter final {
    public:
        using value_type        = ValueType;
        using reference         = value_type&;
        using pointer           = value_type*;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        auto operator*() const -> reference {
            assert(m_current != nullptr);
            return m_current->value;
        }

        auto operator->() const -> pointer {
            assert(m_current != nullptr);
            return &m_current->value;
        }

        auto operator++() -> ReverseIter& { // ++iter
            assert(m_current != nullptr);
            m_current = m_current->prev;
            return *this;
        }

        auto operator++(int) -> ReverseIter { // iter++
            auto temp = *this;
            ++(*this);
            return temp;
        }

        auto operator==(const ReverseIter& rhs) const -> bool {
            return m_current == rhs.m_current;
        }

        auto operator!=(const ReverseIter& rhs) const -> bool {
            return !(*this == rhs);
        }

        // iterator -> const_iterator conversion
        template<typename N = NodeType, typename V = ValueType>
        requires (!std::is_const_v<N>)
        operator ReverseIter<const N, const V>() const {
            return ReverseIter<const N, const V>(m_current);
        }

    private:
        friend class DList;

        explicit ReverseIter(NodeType* current)
            : m_current{current}
        {}

        NodeType* m_current;
    };

    using reverse_iterator = ReverseIter<Node, T>;
    using const_reverse_iterator = ReverseIter<const Node, const T>;

    auto rbegin() -> reverse_iterator {
        return reverse_iterator{m_tail};
    }

    auto rend() -> reverse_iterator {
        return reverse_iterator{nullptr};
    }

    auto rbegin() const -> const_reverse_iterator {
        return const_reverse_iterator{m_tail};
    }

    auto rend() const -> const_reverse_iterator {
        return const_reverse_iterator{nullptr};
    }

private:
    std::unique_ptr<Node> m_head{nullptr};
    Node* m_tail{nullptr};
    std::size_t m_size{0};
};
