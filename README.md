# segmented-list

An array-like data structure that is segmented into blocks for memory efficiency

## How it works

The idea behind this container is to reduce the overhead that comes with `vector` resizes. Since `vector` is _contiguous,_ all existing elements must be copied whenever the container changes sizes. As more resizes occur, the container requires more and more empty space on the end to avoid resizing too frequently. This contributes to slowdowns as well as [heap fragmentation](https://en.wikipedia.org/wiki/Fragmentation_(computing)), leading to all sorts of other problems.

The other option is to utilize non-contiguous storage, such as with `list`. However, this has a number of disadvantages associated with it, namely the lack of random access as well as requiring additional pointers for every individual element to point to the previous and next elements in the list.

This container attempts to offer the best of both worlds; allowing random access at a low performance penalty while maximizing memory efficiency. The container allocates _blocks_ of contiguous memory (like an array) when needed, chained together with pointers (like a linked list). This means that when the container needs to expand, existing elements remain in place, and the allocator requests smaller, consistently-sized chunks of memory at a time rather than creating increasingly larger chunks with each allocation. Further, the container can give back resources it isn't using, unlike a `vector`.

The disadvantage is that the container does not allow for constant-time random access; it supports a bidirectional iterator, not a random-access one because internal blocks of memory are not guaranteed to be contiguous. This means that worst-case lookup time has a complexity of O(n).

## Getting started

As this is a header-only, template container, just `#include "segmented_list.hpp` and you will be good to go. This container follows STL conventions for function names, parameters, etc.. It also includes an `Allocator` parameter for use with custom allocators.

The block size can be configured through template parameters. This *must* be a compile-time constant because the underlying structure is `std::array`.
