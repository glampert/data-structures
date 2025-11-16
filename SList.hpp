#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <optional>
#include <type_traits>

// ----------------------------------------------
// SList: Singly-Linked List
// ----------------------------------------------

template<typename T>
class SList final {
private:
    struct Node {
        T value;
        std::unique_ptr<Node> next;

        template<typename... Args>
        Node(std::unique_ptr<Node>&& next, Args&&... args)
            : value{std::forward<Args>(args)...}
            , next{std::move(next)}
        {}
    };

public:
    template<typename... Args>
    auto emplace_front(Args&&... args) -> void {
        m_head = std::make_unique<Node>(std::move(m_head), std::forward<Args>(args)...);
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
        --m_size;

        return std::optional{std::move(prev_head->value)};
    }

    auto clear() -> void {
        m_head = nullptr;
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
        } else {
            assert(size() != 0);
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

    template<typename NodeType, typename ValueType>
    class ForwardIter final {
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
        friend class SList;

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

private:
    std::unique_ptr<Node> m_head{nullptr};
    std::size_t m_size{0};
};
