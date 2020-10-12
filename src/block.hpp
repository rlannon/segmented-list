#pragma once

/*

block.hpp
Copyright 2020 Riley Lannon

Contains the implementation of the inner block structure for the segmented_list container

*/

#include <array>
#include <type_traits>

template <typename T, size_t N = 21>
class _block
{
    /*

    _block
    Template parameters:
        * T -   The contained type
        * N -   The size of the block (can be configured)

    */

    template <typename, typename > friend class segmented_list;

    std::array<T, N> _arr;

    _block* _previous;
    _block* _next;

    const size_t _capacity;
    size_t _size;
public:
    using value_type = T;
    using reference = value_type & ;
    using pointer = value_type * ;
    using const_reference = const T & ;
    using const_pointer = const T * ;
    using size_type = size_t;

    static const size_type block_size()
    {
        return N;
    }

    size_type capacity() const
    {
        return _capacity;
    }

    size_type size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    void push_back(const T& val)
    {
        /*

        push_back
        Adds the element 'val' at the next available position
        If the array is full, throws an out_of_range exception

        */

        if (_size < _capacity)
        {
            _arr[_size] = val;
            _size += 1;
        }
        else
        {
            throw std::out_of_range("_block");
        }   
    }

    void pop_back()
    {
        /*

        pop_back
        Removes the last element from the array

        */

        if (_size == 0)
        {
            throw std::out_of_range("_block");
        }
        else
        {
            // do not call the destructor explicitly, as the std::array destructor will do that
            // this means the element is left in a valid but indeterminate state
            _size -= 1;
        }
    }

    _block(_block<T, N>* tail)
        : _previous(tail)
        , _next(nullptr)
        , _capacity(N)
        , _size(0)
    {
    }

    _block(const _block<T, N>& other)
        : _arr(other._arr)
        , _previous(other._previous)
        , _next(other._next)
        , _capacity(other._capacity)
        , _size(other._size)
    {
    }

    _block(const _block<T, N>&& other)
        : _arr(other._arr)
        , _previous(other._previous)
        , _next(other._next)
        , _capacity(other._capacity)
        , _size(other._size)
    {
        other = nullptr;
    }

    _block()
        : _capacity(N)
        , _size(0)
        , _previous(nullptr)
        , _next(nullptr)
    {
        // default constructor
    }
};
