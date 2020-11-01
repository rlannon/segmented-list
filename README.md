# segmented-list

An array-like data structure that is segmented into blocks for memory efficiency

## How it works

The idea behind this container is to reduce the overhead that comes with `vector` resizes. Since `vector` is _contiguous,_ all existing elements may need be copied when the container is resized. As more resizes occur, the container requires more and more empty space on the end to avoid resizing too frequently. This contributes to slowdowns as well as [heap fragmentation](https://en.wikipedia.org/wiki/Fragmentation_(computing)), itself leading to all sorts of other problems.

The other option is to utilize non-contiguous storage, such as with `list`. However, this has a number of disadvantages associated with it, namely the lack of random access as well as requiring additional pointers for every individual element to point to the previous and next elements in the list.

This container attempts to offer the best of both worlds; allowing random access at a low performance penalty while maximizing memory efficiency. The container allocates _blocks_ of contiguous memory (like an array) when needed, chained together with pointers (like a linked list). This means that when the container needs to expand, existing elements remain in place, and the allocator requests smaller, consistently-sized chunks of memory at a time rather than creating increasingly larger chunks with each allocation. Further, the container can give back resources it isn't using, unlike a `vector`.

The disadvantage is that the container does not allow for constant-time random access because the internal memory is not contiguous. This means that worst-case lookup time has a complexity of O(n) because it is dependent on the number of blocks present. However, lookup time _within a block_ is O(1), and many elements are skipped at a time because of the block chaining. As such, random access of elements within the structure is a little bit better than with a regular singly- or doubly-linked list. Further, elements towards the front or back of the list may have constant-time access due to possible optimizations (since the head and tail blocks, as well as the number of blocks, are known).

## Getting started

As this is a header-only container, just `#include "segmented_list.hpp` and you will be good to go. This container follows STL conventions for function names, template parameters, etc.. It also includes an `Allocator` parameter for use with custom allocators. Its methods shadow the `std::vector` methods in name and functionality.

The block size can be configured through template parameters. This *must* be a compile-time constant because the underlying structure is `std::array`. It is currently set to 21 until a (more) optimal block size is determined.

An example:

    segmented_list<int> s = {10, 20, 30, 40, 50};   // initialization with an initializer-list
    s.insert(s.begin(), 100, 0);    // insert method
    for (auto it = s.rbegin(); it != s.rend(); it++)    // iterator support
    {
        std::cout << *it << std::endl;
    }
    std::cout << s[9] << std::endl; // supports [] or at()
