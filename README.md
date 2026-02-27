# MIPT C++ Course Homeworks

This repository contains my homeworks from the C++ course at MIPT (FPMI). 

In these tasks, we were not allowed to use standard library containers (like `std::vector` or `std::string`) or standard smart pointers. The goal was to implement them from scratch to understand how memory management and C++ work under the hood.

## Tasks included:
* **SharedPtr** - my implementation of `std::shared_ptr` and `std::weak_ptr`.
* **List & StackAllocator** - a doubly-linked list with a custom allocator.
* **Deque** - an exception-safe deque with custom iterators.
* **String** - a custom string class.
* **BigInteger & Rational** - classes for working with very large numbers.

All code was tested for memory leaks.
