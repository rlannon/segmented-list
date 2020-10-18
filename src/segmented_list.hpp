#pragma once

/*

segmented_list.hpp
Copyright 2020 Riley Lannon

*/

#include <memory>
#include <type_traits>
#include <exception>
#include <iterator>
#include <initializer_list>

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

    // like a linked list, track head and tail nodes
    _block<T>* _head;
    _block<T>* _tail;
    _block<T>* _reserved;   // keep one block reserved upon a deallocation

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

    size_t max_size() const noexcept
    {
        return std::allocator_traits<Allocator>::max_size(_allocator);
    }

    bool empty() const noexcept
    {
        return _size == 0;
    }

    Allocator get_allocator() const noexcept
    {
        return _allocator;
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

    reference front()
    {
        if (_size == 0)
        {
            throw std::out_of_range("segmented_list");
        }
        else
        {
            return _head->_arr[0];
        }
    }

    reference back()
    {
        if (_size == 0)
        {
            throw std::out_of_range("segmented list");
        }
        else
        {
            return _tail->_arr[_tail->size() - 1];
        }
    }

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
            _block<T>* containing_node = nullptr;   // the _block containing the array we want to index

            /*

            Get the proper node

            We can optimize if it's close to the head or tail of the list
            We will also iterate from the front if it's toward the front, or the back if it's toward the back 

            */

            if (block_number == 0)
            {
                containing_node = _head;
            }
            else if (block_number == _num_blocks - 1)
            {
                containing_node = _tail;
            }
            else if (block_number <= _num_blocks / 2)
            {
                // iterate through the linked list until we get to the proper block            
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
                containing_node = current_node;
            }
            else
            {
                // iterate backwards
                auto current_node = _tail;
                size_t num_times = _num_blocks - block_number - 1;
                for (size_t i = 0; i < num_times; i++)
                {
                    if (current_node)
                    {
                        current_node = current_node->_previous;
                    }
                    else
                    {
                        throw std::out_of_range("segmented_list");
                    }
                }
                containing_node = current_node;
            }

            // now, index the block's array to get the element
            if (index_number < containing_node->_size)
            {
                return containing_node->_arr[index_number];
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

    void insert(const_iterator& position, const T& val)
    {
        /*

        insert
        Inserts a single element into the list at 'position'
        
        This will move all subsequent elements back

        */

        // increment the size and allocate another block if needed
        _size++;
        if (_size >= _capacity)
        {
            _alloc_block();
        }

        // utilize reverse iterators to move elements back
        reverse_iterator new_position = rbegin();
        reverse_iterator old_position = ++new_position;

        // continue moving back elements until the element after 'old_position'
        // (i.e., 'new_position') is the position to which we are inserting
        while (old_position.base() != position)
        {
            // move the element back
            *new_position = *old_position;

            // update the iterators
            old_position++;
            new_position++;
        }

        // update the element at 'position'
        *position = val;
    }

    void erase(const_iterator& position)
    {
        /*

        erase
        Removes a single element (at 'position') from the list

        This will move all subsequent elements up one position

        */

        iterator cur = position;
        iterator next = ++cur;  // set 'next' to point to the element after cur via operator++()
        while (next != this->end())
        {
            *cur = *next;
            cur++;
            next++;
        }

        // update the size (and capacity if necessary)
        _size--;
        if (_tail->empty())
        {
            if (_reserved)
            {
                // update the tail node
                _reserved = _tail;
                _tail = _tail->_previous;

                // clear size and chain data from the reserved block
                _reserved->_size = 0;
                _reserved->_previous = nullptr;
                _reserved->_next = nullptr;
            }
            else
            {
                // mark the new end of the list
                auto to_delete = _tail;
                _tail = _tail->_previous;
                _tail->_next = nullptr;

                // destroy and deallocate the old _tail node
                std::allocator_traits<Allocator>::destroy(_allocator, to_delete);
                std::allocator_traits<Allocator>::deallocate(_allocator, to_delete, 1);
            }
        }
    }

    void clear()
    {
        /*

        clear
        Erase all blocks in the container, leaving it with a size and capacity of 0

        */

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

        // if there was a block on reserve, destroy and deallocate that too
        if (_reserved)
        {
            std::allocator_traits<Allocator>::destroy(_allocator, _reserved);
            std::allocator_traits<Allocator>::deallocate(_allocator, _reserved, 1);

            _reserved = nullptr;
        }

        // update our members
        _capacity = 0;
        _size = 0;
        _num_blocks = 0;

        _head = nullptr;
        _tail = nullptr;
    }

    // constructors
    // todo: follow C++20 standards

    explicit segmented_list(const Allocator& alloc) noexcept
        : segmented_list()
        , _allocator(alloc)
    {
    }

    segmented_list(size_t count, const T& value, const Allocator& alloc = Allocator())
        : segmented_list()
        , _allocator(alloc)
    {
        // initialize with 'count' elements all equal to 'value'

        // fill the space with values; the container will automatically be resized when needed
        for (size_t i = 0; i < count; i++)
        {
            this->push_back(value);
        }
    }

    explicit segmented_list(size_t count, const Allocator& alloc = Allocator())
        : _allocator(alloc)
    {
        // initial size of 'count' with default-inserted T
        for (size_t i = 0; i < count; i++)
        {
            this->push_back( T{} );
        }
    }

    segmented_list(const initializer_list<T>& il)
    {
        // initialization with initializer list
        for (auto it = il.begin(); it != il.end(); it++)
        {
            this->push_back(*it);
        }
    }

    segmented_list(const segmented_list& other)
        : _head(other._head)
        , _tail(other._tail)
        , _num_blocks(other._num_blocks)
        , _size(other._size)
        , _capacity(other._capacity)
        , _allocator(other._allocator)
    {
        // copy constructor
    }

    segmented_list(segmented_list&& other) noexcept
        : _head(other._head)
        , _tail(other._tail)
        , _num_blocks(other._num_blocks)
        , _size(other._size)
        , _capacity(other._capacity)
        , _allocator(other._allocator)
    {
        // move constructor
        other._head = nullptr;
        other._tail = nullptr;
        other._num_blocks = 0;
        other._size = 0;
        other._capacity = 0;
        other._allocator = nullptr;
    }

    segmented_list(segmented_list&& other, const Allocator& alloc)
        : _head(other._head)
        , _tail(other._tail)
        , _num_blocks(other._num_blocks)
        , _size(other._size)
        , _capacity(other._capacity)
        , _allocator(alloc)
    {
        // allocator-extended move constructor

        // todo: case where alloc != other.get_allocator()

        other._head = nullptr;
        other._tail = nullptr;
        other._num_blocks = 0;
        other._size = 0;
        other._capacity = 0;
        other._allocator = nullptr;
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
        // deallocate each block by calling 'clear'
        clear();
    }
};
