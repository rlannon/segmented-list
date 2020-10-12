#pragma once

/*

segmented_list.hpp
Copyright 2020 Riley Lannon

*/

#include <memory>
#include <type_traits>
#include <exception>

#include "block.hpp"

template<typename T, typename Allocator = std::allocator<_block<T> > >
class segmented_list
{
    /*

    segmented_list

    Template parameters:
        * T -   The contained type
        * Allocator -    The allocator to use; defaults to allocator<_block<T>>

    */

    _block<T>* _head;
    _block<T>* _tail;
    _block<T>* _reserved;

    Allocator _allocator;

    size_t _capacity;
    size_t _size;

    void _alloc_block()
    {
        /*

        _alloc_block
        Adds a new empty block to the list

        May perform an allocation if necessary. However, if there is a reserved block, it will utilize that.
        
        */

        if (_reserved)
        {
            // update the relationships
            _tail->_next = _reserved;
            _reserved->_previous = _tail;

            // update the tail node
            _tail = _reserved;
        }
        else
        {
            // allocates a new block and appends it
            auto allocated = std::allocator_traits<Allocator>::allocate(_allocator, 1);
            std::allocator_traits<Allocator>::construct(_allocator, allocated, _tail);

            // update _tail to point to this block
            _tail = allocated;
        }
    }
public:
    using value_type = T;
    using reference = T & ;
    using pointer = T * ;
    using const_reference = const T & ;
    using const_pointer = const T * ;
    using size_type = size_t;
    
    value_type at(size_type idx)
    {
        /*

        at
        Returns the element at the specified position

        The algorithm is as follows:
            * Divide the index (idx) by the capacity of the block
            * This will indicate the block number we need
            * The remainder of that division will be its index within that block
            * Iterate through blocks until we find the proper one
            * Index the array in the block

        */

        value_type v;

        if (idx < _size)
        {
            // get the block size and index number
            auto block_number = idx / _block<T>::capacity();
            auto index_number = idx % _block<T>::capacity();
            auto current_node = _head;

            for (size_t i = 0; i < block_number; i++)
            {
                if (current_node)
                {
                    current_node = current_node->_next;
                }
                else
                {
                    throw std::out_of_range("segmented_list");
                }
            }

            if (index_number < current_node->_size)
            {
                v = current_node->_arr[index_number];
            }
            else
            {
                throw std::out_of_range("segmented_list");
            }
            
        }
        else
        {
            throw std::out_of_range("segmented_list");
        }

        return v;   
    }

    void push_back(const T& val)
    {
        /*

        push_back
        Adds an element to the back of the list

        */

        // check to see if allocating another block is necessary
        if (_size == _capacity)
        {
            _alloc_block();
        }
        else
        {
            // insert the new element
            _tail->push_back(val);
            _size += 1; // increase the size
        }
    }

    void pop_back()
    {
        /*

        pop_back
        Removes an element from the back of the list
    
        If the list is empty, throws an exception

        */

        if (_size == 0)
        {
            throw std::out_of_range("segmented_list");
        }
        else
        {
            // decrease the _tail size; if the size is now 0, (maybe) deallocate
            _tail->_size -= 1;
            if (_tail->_size == 0)
            {
                auto new_tail = _tail->_previous;
                new_tail->_next = nullptr;

                if (_reserved)
                {
                    // destroy and deallocate
                    std::allocator_traits<Allocator>::destroy(_allocator, _tail);
                    std::allocator_traits<Allocator>::deallocate(_allocator, _tail, 1);
                }
                else
                {
                    // set this node as our reserve node
                    _reserved = _tail;
                    _reserved->_previous = nullptr;
                    _reserved->_next = nullptr;
                }

                // update the tail node
                _tail = new_tail;
            }
        }
    }

    segmented_list() noexcept
        : _head(nullptr)
        , _tail(nullptr)
        , _reserved(nullptr)
        , _size(0)
        , _capacity(0)
    {
        // default constructor
    }

    ~segmented_list()
    {
        // deallocate each block
        auto current = _head;
        while (current)
        {
            // get the next block (to save the address)
            auto next = current->_next;
            
            // destroy and deallocate the block
            std::allocator_traits<Allocator>::destroy(_allocator, current);
            std::allocator_traits<Allocator>::deallocate(_allocator, current, 1);

            // update the current block
            current = next;
        }
    }
};
