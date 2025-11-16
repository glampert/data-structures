#pragma once

#include "SList.hpp"
#include "DList.hpp"
#include "BTree.hpp"
#include "RBTree.hpp"
#include "BinaryHeap.hpp"
#include "Sorting.hpp"
#include "Graph.hpp"

#include <cstdlib>
#include <string>
#include <print>

// ----------------------------------------------
// SList Tests
// ----------------------------------------------

inline auto slist_tests() -> void {
    std::println("SList Tests:");

    SList<std::string> list{};

    assert(list.is_empty());
    assert(list.size() == 0);

    list.push_front("test 0");
    list.push_front("test 1");
    list.push_front("test 2");

    assert(!list.is_empty());
    assert(list.size() == 3);

    auto test2 = list.pop_front();
    assert(test2 == "test 2");
    assert(list.size() == 2);

    auto test1 = list.pop_front();
    assert(test1 == "test 1");
    assert(list.size() == 1);

    auto test0 = list.pop_front();
    assert(test0 == "test 0");
    assert(list.size() == 0);

    assert(list.is_empty());
    assert(!list.pop_front().has_value());

    for (int i = 0; i < 5; ++i) {
        list.push_front("i=" + std::to_string(i));
    }

    const std::string last{"i=5"};
    list.push_front(last);

    for (const auto& i : list) {
        std::println("List Item: {}", i);
    }

    list.validate();

    // Iterator conversion: iterator -> const_iterator
    [[maybe_unused]] SList<std::string>::iterator it = list.begin();
    [[maybe_unused]] SList<std::string>::const_iterator cit = it;

    list.clear();
    assert(list.is_empty());

    std::print("\n");
}

// ----------------------------------------------
// DList Tests
// ----------------------------------------------

inline auto dlist_tests() -> void {
    std::println("DList Tests:");

    DList<std::string> list{};

    assert(list.is_empty());
    assert(list.size() == 0);

    list.push_front("test 0");
    list.push_front("test 1");
    list.push_front("test 2");

    assert(!list.is_empty());
    assert(list.size() == 3);
    assert(list.front() == "test 2");
    assert(list.back()  == "test 0");

    auto test2 = list.pop_front();
    assert(test2 == "test 2");
    assert(list.size() == 2);
    assert(list.front() == "test 1");
    assert(list.back()  == "test 0");

    auto test1 = list.pop_front();
    assert(test1 == "test 1");
    assert(list.size() == 1);
    assert(list.front() == "test 0");
    assert(list.back()  == "test 0");

    auto test0 = list.pop_front();
    assert(test0 == "test 0");
    assert(list.size() == 0);

    assert(list.is_empty());
    assert(!list.pop_front().has_value());

    for (int i = 0; i < 5; ++i) {
        list.push_front("i=" + std::to_string(i));
    }

    const std::string last{"i=5"};
    list.push_front(last);

    assert(list.front() == last);
    assert(list.back()  == "i=0");

    std::println("Forward Iter:");
    for (const auto& i : list) {
        std::println("List Item: {}", i);
    }

    std::println("Reverse Iter:");
    for (auto iter = list.rbegin(); iter != list.rend(); ++iter) {
        std::println("List Item: {}", *iter);
    }

    list.validate();

    // Iterator conversion: iterator -> const_iterator
    [[maybe_unused]] DList<std::string>::iterator it = list.begin();
    [[maybe_unused]] DList<std::string>::const_iterator cit = it;

    [[maybe_unused]] DList<std::string>::reverse_iterator rit = list.rbegin();
    [[maybe_unused]] DList<std::string>::const_reverse_iterator crit = rit;

    list.clear();
    assert(list.is_empty());

    list.push_front("a");
    list.push_front("b");
    assert(list.front() == "b");

    list.push_back("c");
    list.push_back("d");
    assert(list.back() == "d");

    list.validate();

    std::println("Another DList:");
    int e = 0;
    const char* const expected[] = { "b", "a", "c", "d" };
    for (const auto& i : list) {
        std::println("List Item: {}", i);
        assert(i == expected[e]);
        ++e;
    }

    std::print("\n");
}

// ----------------------------------------------
// BTree Tests
// ----------------------------------------------

inline auto btree_tests() -> void {
    std::println("BTree Tests:");

    using MyBTree = BTree<int, std::string>;

    MyBTree btree{};
    assert(btree.is_empty());
    assert(btree.size() == 0);

    // Insert: 8 -> 3 -> 10 -> 1 -> 6 -> 14 -> 4 -> 7 -> 13
    assert(btree.insert(8,  "a") == MyBTree::Result::Inserted);
    assert(btree.insert(3,  "b") == MyBTree::Result::Inserted);
    assert(btree.insert(10, "c") == MyBTree::Result::Inserted);
    assert(btree.insert(1,  "d") == MyBTree::Result::Inserted);
    assert(btree.insert(6,  "e") == MyBTree::Result::Inserted);
    assert(btree.insert(14, "f") == MyBTree::Result::Inserted);
    assert(btree.insert(4,  "g") == MyBTree::Result::Inserted);
    assert(btree.insert(7,  "h") == MyBTree::Result::Inserted);
    assert(btree.insert(13, "i") == MyBTree::Result::Inserted);
    assert(btree.size() == 9);
    btree.validate();

    // Iterate:
    std::print("Inroder: ");
    btree.visit_inorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Preorder: ");
    btree.visit_preorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Postorder: ");
    btree.visit_postorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Iterator: ");
    for (const auto [key, value] : btree) {
        std::print("[{},{}] -> ", key, value);
    }
    std::print("~\n");

    // Insert exiting key: fails
    assert(btree.insert(1, "x") == MyBTree::Result::Failed);
    assert(btree.size() == 9);

    // Insert or update existing key: ok
    assert(btree.insert_or_update(1, "x") == MyBTree::Result::Updated);
    assert(btree.size() == 9);

    // Search:
    const std::string empty = "";
    assert(btree.find(8).value_or(empty).get()  == "a");
    assert(btree.find(4).value_or(empty).get()  == "g");
    assert(btree.find(1).value_or(empty).get()  == "x");
    assert(btree.find(13).value_or(empty).get() == "i");

    // Remove:
    assert(btree.remove(42).has_value() == false); // Unknown key - fail gracefully.
    btree.validate();
    assert(btree.remove(8).value_or(empty) == "a");
    btree.validate();
    assert(btree.remove(4).value_or(empty) == "g");
    btree.validate();
    assert(btree.remove(1).value_or(empty) == "x");
    btree.validate();
    assert(btree.remove(13).value_or(empty) == "i");
    btree.validate();
    assert(btree.size() == 5);

    // Ensure removed:
    assert(btree.remove(8).has_value() == false);
    btree.validate();
    assert(btree.remove(4).has_value() == false);
    btree.validate();
    assert(btree.remove(1).has_value() == false);
    btree.validate();
    assert(btree.remove(13).has_value() == false);
    btree.validate();

    const int k = 20;
    const std::string v = "hello";
    assert(btree.insert(k, v) == MyBTree::Result::Inserted);
    assert(btree.insert_or_update(k, "world") == MyBTree::Result::Updated);
    btree.validate();

    std::print("BTree: ");
    for (const auto [key, value] : btree) {
        std::print("[{},{}] -> ", key, value);
    }
    std::print("~\n");

    // Clear:
    btree.clear();
    assert(btree.is_empty());
    assert(btree.size() == 0);

    // Iterator conversion: iterator -> const_iterator
    [[maybe_unused]] MyBTree::iterator it = btree.begin();
    [[maybe_unused]] MyBTree::const_iterator cit = it;

    std::print("\n");
}

// ----------------------------------------------
// RBTree Tests
// ----------------------------------------------

inline auto rbtree_tests() -> void {
    std::println("RBTree Tests:");

    using MyRBTree = RBTree<int, std::string>;

    MyRBTree rbtree{};
    assert(rbtree.is_empty());
    assert(rbtree.size() == 0);

    // Insert sorted inputs:
    {
        for (int i = 0; i < 15; ++i) {
            assert(rbtree.insert(i, std::to_string(i)) == MyRBTree::Result::Inserted);
        }
        assert(rbtree.size() == 15);
        rbtree.validate();
        rbtree.clear();
    }

    // Insert random order inputs:
    {
        std::srand(1337);
        for (int i = 0; i < 15; ++i) {
            const int val = i + std::rand();
            assert(rbtree.insert(val, std::to_string(val)) == MyRBTree::Result::Inserted);
        }
        assert(rbtree.size() == 15);
        rbtree.validate();
        rbtree.clear();
    }

    // Insert: 8 -> 3 -> 10 -> 1 -> 6 -> 14 -> 4 -> 7 -> 13
    assert(rbtree.insert(8,  "a") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(3,  "b") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(10, "c") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(1,  "d") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(6,  "e") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(14, "f") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(4,  "g") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(7,  "h") == MyRBTree::Result::Inserted);
    assert(rbtree.insert(13, "i") == MyRBTree::Result::Inserted);
    assert(rbtree.size() == 9);
    rbtree.validate();

    // Iterate:
    std::print("Inroder: ");
    rbtree.visit_inorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Preorder: ");
    rbtree.visit_preorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Postorder: ");
    rbtree.visit_postorder([](const auto& key, const auto& value) {
        std::print("[{},{}] -> ", key, value);
        return true;
    });
    std::print("~\n");

    std::print("Iterator: ");
    for (const auto [key, value] : rbtree) {
        std::print("[{},{}] -> ", key, value);
    }
    std::print("~\n");

    // Insert exiting key: fails
    assert(rbtree.insert(1, "x") == MyRBTree::Result::Failed);
    assert(rbtree.size() == 9);
    rbtree.validate();

    // Insert or update existing key: ok
    assert(rbtree.insert_or_update(1, "x") == MyRBTree::Result::Updated);
    assert(rbtree.size() == 9);
    rbtree.validate();

    // Search:
    const std::string empty = "";
    assert(rbtree.find(8).value_or(empty).get()  == "a");
    assert(rbtree.find(4).value_or(empty).get()  == "g");
    assert(rbtree.find(1).value_or(empty).get()  == "x");
    assert(rbtree.find(13).value_or(empty).get() == "i");

    // Remove:
    assert(rbtree.remove(42).has_value() == false); // Unknown key - fail gracefully.
    rbtree.validate();
    assert(rbtree.remove(8).value_or(empty) == "a");
    rbtree.validate();
    assert(rbtree.remove(4).value_or(empty) == "g");
    rbtree.validate();
    assert(rbtree.remove(1).value_or(empty) == "x");
    rbtree.validate();
    assert(rbtree.remove(13).value_or(empty) == "i");
    rbtree.validate();
    assert(rbtree.size() == 5);

    // Ensure removed:
    assert(rbtree.remove(8).has_value() == false);
    rbtree.validate();
    assert(rbtree.remove(4).has_value() == false);
    rbtree.validate();
    assert(rbtree.remove(1).has_value() == false);
    rbtree.validate();
    assert(rbtree.remove(13).has_value() == false);
    rbtree.validate();

    const int k = 20;
    const std::string v = "hello";
    assert(rbtree.insert(k, v) == MyRBTree::Result::Inserted);
    assert(rbtree.insert_or_update(k, "world") == MyRBTree::Result::Updated);
    rbtree.validate();

    std::print("RBTree: ");
    for (const auto [key, value] : rbtree) {
        std::print("[{},{}] -> ", key, value);
    }
    std::print("~\n");

    // Clear:
    rbtree.clear();
    rbtree.validate();
    assert(rbtree.is_empty());
    assert(rbtree.size() == 0);

    // Iterator conversion: iterator -> const_iterator
    [[maybe_unused]] MyRBTree::iterator it = rbtree.begin();
    [[maybe_unused]] MyRBTree::const_iterator cit = it;

    std::print("\n");
}

// ----------------------------------------------
// BinaryHeap Tests
// ----------------------------------------------

inline auto binary_heap_tests() -> void {
    std::println("BinaryHeap Tests:");

    const std::vector<int> values = {7, 3, 9, 1, 6, 4, 2};

    // Min/Max heap from array:
    {
        BinaryHeap<int, std::less<int>> min_heap{values};
        assert(min_heap.size() == values.size());
        const int top_min = min_heap.peek().value();
        assert(top_min == 1);
        min_heap.validate();

        assert(min_heap.update(0, 5).value() == 1);
        assert(min_heap.peek().value() == 2);
        min_heap.validate();

        std::print("MinHeap: ");
        for (const auto value : min_heap) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");
    }
    {
        BinaryHeap<int, std::greater<int>> max_heap{values};
        assert(max_heap.size() == values.size());
        const int top_max = max_heap.peek().value();
        assert(top_max == 9);
        max_heap.validate();

        assert(max_heap.update(0, 5).value() == 9);
        assert(max_heap.peek().value() == 7);
        max_heap.validate();

        std::print("MaxHeap: ");
        for (const auto value : max_heap) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");
    }

    // Min/Max heap incrementally built:
    {
        BinaryHeap<int, std::less<int>> min_heap{};
        for (int i : values) {
            min_heap.push(i);
        }
        assert(min_heap.size() == values.size());
        const int top_min = min_heap.peek().value();
        assert(top_min == 1);
        min_heap.validate();

        std::println("Pop 1,2");
        auto result = min_heap.pop();
        assert(result.has_value() && result.value() == 1);
        result = min_heap.pop();
        assert(result.has_value() && result.value() == 2);
        min_heap.validate();
        
        std::print("MinHeap: ");
        for (const auto value : min_heap) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");
    }
    {
        BinaryHeap<int, std::greater<int>> max_heap{};
        for (int i : values) {
            max_heap.push(i);
        }
        assert(max_heap.size() == values.size());
        const int top_max = max_heap.peek().value();
        assert(top_max == 9);
        max_heap.validate();

        std::println("Pop 9,7");
        auto result = max_heap.pop();
        assert(result.has_value() && result.value() == 9);
        result = max_heap.pop();
        assert(result.has_value() && result.value() == 7);
        max_heap.validate();
        
        std::print("MaxHeap: ");
        for (const auto value : max_heap) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");
    }

    std::print("\n");
}

// ----------------------------------------------
// Heap Sort Tests
// ----------------------------------------------

inline auto heap_sort_tests() -> void {
    std::println("HeapSort Tests:");

    // Sort ascending:
    {
        std::vector<int> values = {7, 3, 9, 1, 6, 4, 2};

        heap_sort(std::span{values}, std::less<int>{});

        std::print("HeapSort ascending: ");
        for (const auto value : values) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");

        assert(std::is_sorted(values.begin(), values.end(), std::less<int>{}));
    }

    // Sort descending:
    {
        std::vector<int> values = {7, 3, 9, 1, 6, 4, 2};

        heap_sort(std::span{values}, std::greater<int>{});

        std::print("HeapSort descending: ");
        for (const auto value : values) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");

        assert(std::is_sorted(values.begin(), values.end(), std::greater<int>{}));
    }

    // Short sort (2 items), ascending:
    {
        std::vector<int> values = {7, 3};

        heap_sort(std::span{values});

        std::print("HeapSort short-sort: ");
        for (const auto value : values) {
            std::print("[{}] -> ", value);
        }
        std::print("~\n");

        assert(std::is_sorted(values.begin(), values.end()));
    }

    std::print("\n");
}

// ----------------------------------------------
// Graph Tests
// ----------------------------------------------

inline auto graph_tests() -> void {
    std::println("Graph Tests:");

    Graph<int> g{false};
    g.add_edge(1, 2);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    g.add_edge(3, 5);
    assert(!g.has_cycle());
    
    std::print("Graph DFS: ");
    g.visit_depth_first(1, [](const auto& v) {
        std::print("[{}] -> ", v);
        return true;
    });
    std::print("~\n");
    
    std::print("Graph BFS: ");
    g.visit_breadth_first(1, [](const auto& v) {
        std::print("[{}] -> ", v);
        return true;
    });
    std::print("~\n");
    
    Graph<int> undirected{false};
    undirected.add_edge(1, 2);
    undirected.add_edge(2, 3);
    undirected.add_edge(3, 1);
    assert(undirected.has_cycle());

    Graph<int> directed{true};
    directed.add_edge(1, 2);
    directed.add_edge(2, 3);
    directed.add_edge(3, 1);
    assert(directed.has_cycle());

    // Path finding:
    Graph<std::string> gs{true}; // directed

    gs.add_edge("A", "B");
    gs.add_edge("A", "C");
    gs.add_edge("C", "D");
    gs.add_edge("B", "D");

    std::print("Dijkstra Path: ");
    const auto path = gs.dijkstra_shortest_path("A", "D");
    for (const auto& p : path) {
        std::print("[{}] -> ", p);
    }
    std::print("~\n");

    std::print("\n");
}
