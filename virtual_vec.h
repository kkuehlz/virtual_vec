#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {

// Utility class that implements basic operations on initialized and
// uninitialized blocks of memory of type T.
template<typename T>
struct Uninitialized {

Uninitialized() = delete;
Uninitialized(const Uninitialized&) = delete;
Uninitialized& operator=(const Uninitialized&) = delete;
~Uninitialized() = delete;

template<class InputIterator, class OutputIterator>
static inline void copy(InputIterator first, InputIterator last, OutputIterator d_first) {
    for (; first != last; first++, d_first++) {
        d_first->~T();
        new (d_first) T(*first);
    }
}

template<class InputIterator, class OutputIterator>
static inline void move(InputIterator first, InputIterator last, OutputIterator d_first) {
    for (; first != last; first++, d_first++) {
        d_first->~T();
        new (d_first) T(std::move(*first));
    }
}

template<class InputIterator, class OutputIterator>
static inline void move_backward(InputIterator first, InputIterator last, OutputIterator d_last) {
    for (; first != last; last--, d_last--) {
        d_last->~T();
        new (d_last) T(std::move(*last));
    }
}

template<class OutputIterator, class Size>
static inline OutputIterator fill_n(OutputIterator first, Size count, const T& value)
{
    for (Size i = 0; i < count; i++, first++) {
        first->~T();
        new (first) T(value);
    }
    return first;
}

};

}  // namespace

class Memory {
public:
    Memory() = default;
    ~Memory();
    Memory(const Memory& other) = delete;
    Memory& operator=(const Memory& other) = delete;

    static inline size_t avail_mem() {
        constexpr size_t addresses = (4ULL << 30);
        return addresses;
    }

    Memory(Memory&& other) noexcept
          : memory_(other.memory_),
            num_bytes_(other.num_bytes_) {
        other.memory_ = nullptr;
        other.num_bytes_ = 0;
    }

    Memory& operator=(Memory&& other) {
        memory_ = other.memory_;
        num_bytes_ = other.num_bytes_;
        other.memory_ = nullptr;
        other.num_bytes_ = 0;
        return *this;
    }

    void shrink(size_t wanted);
    void grow(size_t wanted);
    inline void grow() { grow(num_bytes() * 2); }

    inline size_t num_bytes() const { return num_bytes_ ; }
    inline uint8_t* pointer() const { return memory_; };

private:
    void reserve();

    uint8_t* memory_ = nullptr;
    size_t num_bytes_ = 0;
};

template <typename T>
class virtual_vec {
 public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;

    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 public:
    virtual_vec() = default;
    ~virtual_vec() { clear(); }

    explicit virtual_vec(size_type count, const T& value) : count_(count) { init_empty_fill(count, value); }
    explicit virtual_vec(size_type count) : count_(count)                 { init_empty_fill(count, T()); }

    virtual_vec(const virtual_vec& other) : count_(other.count_) {
        reserve(other.capacity());
        Uninitialized<T>::copy(other.begin(), other.end(), begin());
    }

    virtual_vec& operator=(const virtual_vec& other) {
        reserve(other.capacity());
        count_ = other.count_;
        Uninitialized<T>::copy(other.begin(), other.end(), begin());
        return *this;
    }

    virtual_vec(virtual_vec&& other) noexcept
        : memory_(std::move(other.memory_)),
          count_(other.count_) {
        other.count_ = 0;
    }

    virtual_vec& operator=(virtual_vec&& other) {
        reserve(other.capacity());
        memory_ = std::move(other.memory_);
        count_ = other.count_;
        other.count_ = 0;
        return *this;
    }

    virtual_vec& operator=(std::initializer_list<T> init) {
        init_empty_move(init.begin(), init.end());
        return *this;
    }

    inline virtual_vec(std::initializer_list<T> init)      { init_empty_move(init.begin(), init.end()); }

    inline reference front()                               { return *begin(); }
    inline const_reference front()                   const { return *cbegin(); }
    inline reference back()                                { return *std::prev(end()); }
    inline const_reference back()                    const { return *std::prev(cend()); }
    inline reference       operator[](size_type pos)       { return memory_ptr()[pos]; }
    inline const_reference operator[](size_type pos) const { return memory_ptr()[pos]; }
    inline T* data()                              noexcept { return memory_ptr(); }
    inline const T* data()                  const noexcept { return memory_ptr(); }

    [[nodiscard]] inline bool empty()       const noexcept { return size() == 0; }
    inline size_type size()                 const noexcept { return count_; }
    inline size_type max_size()             const noexcept { return Memory::avail_mem() / sizeof(T); }
    inline size_type capacity()             const noexcept { return capacity_in_bytes() / sizeof(T); }
    inline void reserve(size_type new_cap)                 { reserve_in_bytes(new_cap * sizeof(T)); }
    void shrink_to_fit() {
        auto new_size = size() * sizeof(T);
        deinit_until(new_size);
        return memory_.shrink(new_size);
    }

    inline iterator begin()                 const noexcept { return memory_ptr(); }
    inline const_iterator cbegin()          const noexcept { return memory_ptr(); }
    inline iterator end()                   const noexcept { return &memory_ptr()[size()]; }
    inline const_iterator cend()            const noexcept { return &memory_ptr()[size()]; }
    inline reverse_iterator rbegin()        const noexcept { return std::reverse_iterator<iterator>(end()); }
    inline const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator<const_iterator>(end()); }
    inline reverse_iterator rend()          const noexcept { return std::reverse_iterator<iterator>(begin()); }
    inline const_reverse_iterator crend()   const noexcept { return std::reverse_iterator<const_iterator>(begin()); }

    reference at(size_type pos) {
        if (!(pos < size())) {
            throw std::out_of_range("Out of bounds");
        }
        return this->operator[](pos);
    }

    const_reference at(size_type pos) const {
        if (!(pos < size())) {
            throw std::out_of_range("Out of bounds");
        }
        return this->operator[](pos);
    }

    inline iterator erase(iterator pos) { return erase(pos, std::next(pos)); }

    iterator erase(const_iterator first, const_iterator last) {
        iterator _first = &this->operator[](std::distance(cbegin(), first));
        iterator _last = &this->operator[](std::distance(cbegin(), last));
        auto leftover = std::move(_last, end(), _first);
        // de-initialize any remaining elements.
        for (; leftover != end(); leftover++) leftover->~T();
        count_ -= std::distance(first, last);
        return _first;
    }

    inline void clear()                              noexcept  { deinit_range(begin(), end()); count_ = 0; }
    inline void push_back(const value_type& value)             { emplace_back(value); }
    inline void push_back(value_type&& value)                  { emplace_back(std::forward<T>(value));}
    inline void pop_back()                                     { erase(std::prev(end())); }
    inline iterator insert(const_iterator pos, const T& value) { emplace(pos, value); }
    inline iterator insert(const_iterator pos, T&& value)      { emplace(pos, std::forward<T>(value)); }
    inline void swap(virtual_vec& other)              noexcept { std::swap(count_, other.count_); std::swap(memory_, other.memory_); }
    inline void resize(size_type count)                        { resize(count, T()); }

    void resize(size_type count, const value_type& value) {
        if (size() < count) {
            reserve(count);
            Uninitialized<T>::fill_n(end(), count - size(), value);
        } else if (size() > count) {
            deinit_from(count);
        }
        count_ = count;
    }

    template<class... Args>
    reference emplace_back(Args&&... args) {
        reserve(size() + 1);
        T* ptr = &memory_ptr()[count_];
        new (ptr) T(std::forward<Args>(args)...);
        count_ += 1;
        return *ptr;
    }

    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        if (empty()) {
            emplace_back(args...);
            return begin();
        }
        iterator _pos = move_right_by(pos, 1);
        *_pos = T(std::forward<Args>(args)...);
        return _pos;
    }

    template<class InputIterator>
    iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
        if (empty()) { return init_empty_copy(first, last); }
        size_type new_elems = std::distance(first, last);
        iterator _pos = move_right_by(pos, new_elems);
        Uninitialized<T>::copy(first, last, _pos);
        return _pos;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> init) {
        if (empty()) { return init_empty_move(init.begin(), init.end()); }
        size_type new_elems = init.size();
        iterator _pos = move_right_by(pos, new_elems);
        Uninitialized<T>::move(init.begin(), init.end(), _pos);
        return _pos;
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        if (empty()) { throw std::runtime_error("not implemented"); }
        iterator _pos = move_right_by(pos, count);
        Uninitialized<T>::fill_n(_pos, count, value);
        return _pos;
    }

 private:
  constexpr inline bool needs_deinit()                            { return !std::is_trivial<T>::value; }
  inline T* memory_ptr()                           const noexcept { return reinterpret_cast<T*>(memory_.pointer()); }
  inline size_type capacity_in_bytes()             const noexcept { return memory_.num_bytes(); }
  inline void reserve_in_bytes(size_type new_cap)                 { if (capacity_in_bytes() < new_cap) memory_.grow(new_cap); }

  inline void deinit_range(iterator start, iterator end)          { if (needs_deinit()) for (; start != end; start++) start->~T(); }
  inline void deinit_from(iterator start)                         { deinit_range(start, end()); }
  inline void deinit_from(size_type offset)                       { deinit_range(&this->operator[](offset), end()); }
  inline void deinit_until(iterator end)                          { deinit_range(begin(), end); }
  inline void deinit_until(size_type offset)                      { deinit_range(begin(), &this->operator[](offset)); }

  inline void init_empty_fill(size_type count, const T& value) {
      reserve(count);
      Uninitialized<T>::fill_n(begin(), size(), value);
  }

  template<typename Iterator>
  iterator move_right_by(Iterator pos, size_type n) {
      reserve(size() + n);
      iterator _pos = &this->operator[](std::distance(cbegin(), pos));
      count_ += n;
      Uninitialized<T>::move_backward(std::prev(_pos), std::prev(end(), n + 1), std::prev(end()));
      return _pos;
  }

  template<typename Iterator>
  iterator init_empty_move(Iterator first, Iterator last) {
      size_t count = std::distance(first, last);
      reserve(count);
      Uninitialized<T>::move(first, last, begin());
      count_ = count;
      return begin();
  }

  template<typename Iterator>
  iterator init_empty_copy(Iterator first, Iterator last) {
      size_t count = std::distance(first, last);
      reserve(count);
      Uninitialized<T>::copy(first, last, begin());
      count_ = count;
      return begin();
  }

  Memory memory_;
  std::size_t count_ = 0;
};
