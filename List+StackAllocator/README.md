# List and StackAllocator

My implementation of a doubly-linked list (`List`) and a custom allocator (`StackAllocator`).

**Key features:**
* `StackAllocator` uses stack memory to make the list work faster without dynamic memory allocations (`new`/`delete`).
* The `List` works correctly with custom allocators (AllocatorAware).
* Bidirectional iterators are supported.
* Exception safety (no memory leaks).
