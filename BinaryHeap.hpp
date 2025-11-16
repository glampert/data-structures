#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <optional>
#include <type_traits>
#include <vector>

// ----------------------------------------------
// Binary Heap / Priority Queue:
// ----------------------------------------------

// Sorted vector as a binary heap / priority-queue.
// push/pop: O(log n)
// update: O(log n)
// peek top: O(1)
// construct from vector: O(n)
template<typename T, typename Compare = std::less<T>>
class BinaryHeap final {
public:
    BinaryHeap() = default;

    explicit BinaryHeap(const Compare& comp)
        : m_comp{comp}
    {}

    explicit BinaryHeap(Compare&& comp)
        : m_comp{std::move(comp)}
    {}

    explicit BinaryHeap(const std::vector<T>& values,
                        const Compare& comp = Compare{})
        : m_heap{values}
        , m_comp{comp}
    {
        heapify();
    }

    explicit BinaryHeap(std::vector<T>&& values,
                        Compare&& comp = Compare{})
        : m_heap{std::move(values)}
        , m_comp{std::move(comp)}
    {
        heapify();
    }

    template<typename... Args>
    auto emplace(Args&&... args) -> void {
        m_heap.emplace_back(std::forward<Args>(args)...);
        bubble_up(m_heap.size() - 1);
    }

    template<typename U>
    auto push(U&& value) -> void
        requires std::constructible_from<T, U>
    {
        emplace(std::forward<U>(value));
    }

    auto pop() -> std::optional<T> {
        if (is_empty()) {
            return std::nullopt;
        }

        std::swap(m_heap.front(), m_heap.back());

        std::optional removed_value{std::move(m_heap.back())};
        m_heap.pop_back();

        if (!is_empty()) {
            bubble_down(0);
        }

        return removed_value;
    }

    // Updates `index` to `value`; returns old value.
    template<typename U>
    auto update(const std::size_t index, U&& value) -> std::optional<T>
        requires std::constructible_from<T, U>
    {
        if (index >= m_heap.size()) {
            return std::nullopt;
        }

        std::optional old_value{std::move(m_heap[index])};
        m_heap[index] = std::forward<U>(value);

        // If new value is better than parent -> move up:
        if (index > 0 && m_comp(m_heap[index], m_heap[parent(index)])) {
            bubble_up(index);
        } else {
            // Otherwise, bubbling down is sufficient.
            bubble_down(index);
        }

        return old_value;
    }

    using ValueConstRef = std::reference_wrapper<const T>;

    auto peek() const -> std::optional<ValueConstRef> {
        if (is_empty()) {
            return std::nullopt;
        }
        return std::optional{std::ref(m_heap[0])};
    }

    auto validate() const -> void {
        const auto n = m_heap.size();

        for (std::size_t i = 0; i < n; ++i) {
            const auto l = left(i);
            const auto r = right(i);

            if (l < n && m_comp(m_heap[l], m_heap[i])) {
                // If left child is "better" than parent -> violation
                assert(false && "Heap invariant violated: left child better than parent");
            }

            // Same check for the right child.
            if (r < n && m_comp(m_heap[r], m_heap[i])) {
                assert(false && "Heap invariant violated: right child better than parent");
            }
        }
    }

    auto is_empty() const -> bool {
        return m_heap.empty();
    }

    auto size() const -> std::size_t {
        return m_heap.size();
    }

    auto begin() const {
        return m_heap.begin();
    }

    auto end() const {
        return m_heap.end();
    }

    auto rbegin() const {
        return m_heap.rbegin();
    }

    auto rend() const {
        return m_heap.rend();
    }

private:
    static auto parent(const std::size_t i) -> std::size_t {
        return (i - 1) / 2;
    }

    static auto left(const std::size_t i) -> std::size_t {
        return (2 * i) + 1;
    }

    static auto right(const std::size_t i) -> std::size_t {
        return (2 * i) + 2;
    }

    auto bubble_up(std::size_t i) -> void {
        while (i != 0) {
            const auto p = parent(i);

            // If heap invariant holds: parent <= child (for min-heap).
            // m_comp(child, parent) means "child < parent".
            if (!m_comp(m_heap[i], m_heap[p])) {
                break;
            }

            std::swap(m_heap[i], m_heap[p]);
            i = p;
        }
    }

    auto bubble_down(std::size_t i) -> void {
        const auto n = m_heap.size();

        // Heap sort:
        for (;;) {
            const auto l = left(i);
            const auto r = right(i);
            auto best = i;

            if (l < n && m_comp(m_heap[l], m_heap[best])) {
                best = l;
            }

            if (r < n && m_comp(m_heap[r], m_heap[best])) {
                best = r;
            }

            // If no child is better, stop.
            if (best == i) {
                break;
            }

            std::swap(m_heap[i], m_heap[best]);
            i = best;
        }
    }

    auto heapify() -> void {
        if (m_heap.empty()) {
            return;
        }

        for (auto i = (m_heap.size() / 2); i-- > 0;) {
            bubble_down(i);
        }
    }

private:
    std::vector<T> m_heap{};

    // If Compare is an empty class, prevent it from adding a dummy byte to the class size.
    [[no_unique_address]]
    Compare m_comp{}; // true if a < b for min-heap (default), or a > b for max-heap.
};

template<typename T, typename Compare = std::less<T>>
using PQueue = BinaryHeap<T, Compare>;
