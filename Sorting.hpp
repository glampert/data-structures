#pragma once

#include <algorithm>
#include <utility>
#include <span>

// ----------------------------------------------
// Heap Sort:
// ----------------------------------------------

template<typename T, typename Compare = std::less<T>>
auto heap_sort(std::span<T> items, const Compare& comp = Compare{}) -> void
    requires(!std::is_const_v<T>)
{
    const auto n = items.size();
    if (n <= 1) {
        return;
    }

    auto bubble_down = [](std::span<T> heap, const Compare& comp, const std::size_t start, const std::size_t end) {
        for (auto i = start;;) {
            const auto left  = (2 * i) + 1;
            const auto right = (2 * i) + 2;
            auto best = i;

            if (left < end && comp(heap[best], heap[left])) {
                best = left;
            }

            if (right < end && comp(heap[best], heap[right])) {
                best = right;
            }

            // If no child is better, stop.
            if (best == i) {
                break;
            }

            std::swap(heap[i], heap[best]);
            i = best;
        }
    };

    // 1) Build heap: (max-heap when comp = std::less)
    for (auto i = static_cast<std::int64_t>((n - 2) / 2); i >= 0; --i) {
        bubble_down(items, comp, static_cast<std::size_t>(i), n);
    }

    // 2) Repeatedly extract-max and shrink the heap:
    for (auto end = static_cast<std::int64_t>(n - 1); end > 0; --end) {
        std::swap(items[0], items[static_cast<std::size_t>(end)]); // Move root to end
        bubble_down(items, comp, 0, static_cast<std::size_t>(end));
    }
}
