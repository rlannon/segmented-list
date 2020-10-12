#pragma once

/*

segmented_list.hpp
Copyright 2020 Riley Lannon

*/

#include <memory>
#include <type_traits>
#include <exception>
#include <iterator>

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
    size_t _num_blocks;

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

            if (_num_blocks == 0)
            {
                _head = allocated;
            }
            else if (_num_blocks == 1)
            {
                _head->_next = allocated;
            }
            else
            {
                _tail->_next = allocated;
            }

            _tail = allocated;
        }

        _capacity += _block<T>::block_size();
        _num_blocks++;
    }
public:
    // define other class traits
    using value_type = T;
    using reference = T & ;
    using pointer = T * ;
    using const_reference = const T & ;
    using const_pointer = const T * ;
    using size_type = size_t;
    using allocator_type = Allocator;

    size_t size() const noexcept
    {
        return _size;
    }

    size_t capacity() const noexcept
    {
        return _capacity;
    }

    bool empty() const noexcept
    {
        return _size == 0;
    }

    template <bool is_const = false>
    class list_iterator
    {
        friend class segmented_list;    // ensure the parent class is a friend

        _block<value_type>* _block_pointer;  // pointer to the block we are in
        size_t _elem_index; // index within the block

        list_iterator(_block<value_type>* p, size_t idx)
            : _block_pointer(p)
            , _elem_index(idx)
        {
            // private constructor
        }
    public:
        using value_type = typename std::conditional<is_const, const T, T>::type;
        using pointer = value_type * ;
        using reference = value_type & ;
        using const_pointer = const T * ;
        using const_reference = const T & ;
        using iterator_category = std::bidirectional_iterator_tag;

        // Operator overloads

        bool operator==(const list_iterator& right)
        {
            return _block_pointer == right._block_pointer && _elem_index == right._elem_index;
        }

        bool operator!=(const list_iterator& right)
        {
            return _block_pointer != right._block_pointer || _elem_index != right._elem_index;
        }

        list_iterator::reference operator*()
        {
            if (
                (_block_pointer != nullptr) && 
                (_elem_index < (_block_pointer->capacity()) ) 
            )
            {
                return _block_pointer->_arr[_elem_index];
            }
            else
            {
                throw std::out_of_range("segmented_list iterator");
            }
        }

        list_iterator::pointer operator->()
        {
            if (_block_pointer && (_elem_index < (_block_pointer->capacity()) ) )
            {
                return &_block_pointer->_arr[_elem_index];
            }
            else
            {
                throw std::out_of_range("segmented_list iterator");
            }
            
        }

        list_iterator& operator++()
        {
            if (_block_pointer)
            {
                _elem_index++;

                // check to see if we need to move to the next block
                if (_elem_index == _block<value_type>::block_size())
                {
                    _elem_index = 0;
                    _block_pointer = _block_pointer->_next;
                }

                // check to see if we are now past the end
                if (_block_pointer && _elem_index >= _block_pointer->_size)
                {
                    _block_pointer = nullptr;
                    _elem_index = 0;
                }
            }
            else
            {
                throw std::out_of_range("segmented_list iterator");
            }

            return *this;
        }

        list_iterator operator++(int)
        {
            list_iterator li { *this };

            if (_block_pointer)
            {
                _elem_index++;

                // check to see if we need to move onto the next block
                if (_elem_index == _block<value_type>::block_size())
                {
                    _elem_index = 0;
                    _block_pointer = _block_pointer->_next;
                }

                // check to see if we are now past the end
                if (_block_pointer && _elem_index >= _block_pointer->_size)
                {
                    _block_pointer = nullptr;
                    _elem_index = 0;
                }
            }
            else
            {
                throw std::out_of_range("segmented_list iterator");
            }
            
            return li;
        }

        list_iterator& operator--()
        {
            if (_block_pointer)
            {
                if (_elem_index == 0)
                {
                    _elem_index = _block<value_type>::block_size() - 1;
                    _block_pointer = _block_pointer->_previous;

                    // if there is no previous block, ensure elem_index is set to 0
                    if (_block_pointer == nullptr)
                        _elem_index = 0;
                }
                else
                {
                    _elem_index--;
                }
            }
            else
            {
                // this points to the past-the-end element
                // todo: determine how to handle the past-the-end element (and before-the-beginning)
            }
            
            return *this;
        }

        list_iterator operator--(int)
        {
            list_iterator li { *this };

            if (_block_pointer)
            {
                if (_elem_index == 0)
                {
                    _elem_index = _block<value_type>::block_size() - 1;
                    _block_pointer = _block_pointer->_previous;

                    // if there is no previous block, ensure elem_index is set to 0
                    if (_block_pointer == nullptr)
                        _elem_index = 0;
                }
                else
                {
                    _elem_index--;
                }
            }
            else
            {
                // this points to the past-the-end element
                // todo: determine how to handle these cases
            }

            return li;
        }

        template<bool _is_const = is_const,
            typename std::enable_if<_is_const, int>::value = 1>
        list_iterator& operator=(const list_iterator<false>& it)
        {
            // converting copy assignment operator
            _block_pointer = it._block_pointer;
            _elem_index = it._elem_index;
        }

        template<bool _is_const = is_const,
            typename std::enable_if<_is_const, int>::value = 1>
        list_iterator& operator=(list_iterator<false>&& it)
        {
            // converting move assignment operator
            _block_pointer = it._block_pointer;
            _elem_index = it._elem_index;
            
            it._block_pointer = nullptr;
            it._elem_index = 0;
        }

        list_iterator& operator=(const list_iterator& it)
        {
            // default copy assignment operator
            _block_pointer = it._block_pointer;
            _elem_index = it._elem_index;
        }

        list_iterator& operator=(list_iterator&& it)
        {
            // default move assignment operator
            _block_pointer = it._block_pointer;
            _elem_index = it._elem_index;
            
            it._block_pointer = nullptr;
            it._elem_index = 0;
        }

        // Constructors

        list_iterator() noexcept
            : _block_pointer(nullptr)
            , _elem_index(0) {}

        template<bool _is_const = is_const,
            typename std::enable_if<_is_const, int>::value = 1>
        list_iterator(const list_iterator<false>& it)
            : _block_pointer(it._block_pointer)
            , _elem_index(it._elem_index)
        {
            // converting copy constructor
        }

        template<bool _is_const = is_const,
            typename std::enable_if<_is_const, int>::value = 1>
        list_iterator(list_iterator<false>&& it)
            : _block_pointer(it._block_pointer)
            , _elem_index(it._elem_index)
        {
            // converting move constructor
            it._block_pointer = nullptr;
            it._elem_index = 0;
        }

        list_iterator(const list_iterator& it)
            : _block_pointer(it._block_pointer)
            , _elem_index(it._elem_index)
        {
            // default copy constructor
        }

        list_iterator(list_iterator&& it)
            : _block_pointer(it._block_pointer)
            , _elem_index(it._elem_index)
        {
            // default move constructor
            it._block_pointer = nullptr;
            it._elem_index = 0;
        }
    };

    // define our iterators
    using iterator = list_iterator<false>;
    using const_iterator = list_iterator<true>;
    using reverse_iterator = std::reverse_iterator<list_iterator<false> >;
    using const_reverse_iterator = std::reverse_iterator<list_iterator<true> >;

    iterator begin()
    {
        if (_size == 0)
        {
            // todo: how to handle before-the-beginning element?
            return iterator(nullptr, 0);
        }
        else
        {
            // return an iterator to the first element
            return iterator(_head, 0);
        }
        
    }

    const_iterator cbegin()
    {
        if (_size == 0)
        {
            // todo: how to handle?
            return const_iterator(nullptr, 0);
        }
        else
        {
            return const_iterator(_head, 0);
        }
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(_tail, _tail->size() - 1);
    }

    const_reverse_iterator crbegin()
    {
        return const_reverse_iterator(_tail, _tail->size() - 1);
    }

    iterator end()
    {
        // todo: how to handle past-the-end?
        return iterator(nullptr, 0);
    }

    const_iterator cend()
    {
        return const_iterator(nullptr, 0);
    }

    reverse_iterator rend()
    {
        return reverse_iterator(nullptr, 0);
    }

    const_reverse_iterator crend()
    {
        return const_reverse_iterator(nullptr, 0);
    }
    
    reference at(size_type n)
    {
        /*

        at
        Returns the element at the specified position

        The algorithm is as follows:
            * Divide the index (n) by the capacity of the block
            * This will indicate the block number we need
            * The remainder of that division will be its index within that block
            * Iterate through blocks until we find the proper one
            * Index the array in the block

        */

        if (n < _size)
        {
            // get the block size and index number
            auto block_number = n / _block<value_type>::block_size();
            auto index_number = n % _block<value_type>::block_size();
            auto current_node = _head;

            // iterate through the linked list until we get to the proper block
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
                return current_node->_arr[index_number];
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
    }

    reference operator[](size_t n)
    {
        // an alias for 'at'
        return this->at(n);
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
        
        // insert the new element
        _tail->push_back(val);
        _size += 1; // increase the size
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
            // decrease the _tail size
            _tail->_size -= 1;
            _size--;

            // if the size is zero, decrease the capacity
            if (_tail->_size == 0)
            {
                auto new_tail = _tail->_previous;
                new_tail->_next = nullptr;

                // if we have a block reserved already, deallocate this block
                // otherwise, use _this_ block as our block in reserve
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
                    _reserved->_size = 0;
                }

                // update the tail node and the capacity
                _tail = new_tail;
                _capacity -= _block<T>::block_size();
                _num_blocks--;
            }
        }
    }

    segmented_list() noexcept
        : _head(nullptr)
        , _tail(nullptr)
        , _reserved(nullptr)
        , _size(0)
        , _capacity(0)
        , _num_blocks(0)
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
