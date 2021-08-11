#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#include <iostream> // fixme: remove

class Memory {
public:
    Memory();

    void resize(size_t wanted);
    inline void resize() { resize(num_bytes() * 2); }

    inline size_t num_bytes() const { return num_bytes_ ; }
    inline uint8_t* pointer() const { return memory_; };

private:
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

    inline size_type size()                 const noexcept { return count_; }
    inline size_type capacity()             const noexcept { return memory_.num_bytes(); }

    inline iterator begin()                 const noexcept { return memory_ptr(); }
    inline const_iterator cbegin()          const noexcept { return memory_ptr(); }
    inline iterator end()                   const noexcept { return &memory_ptr()[size()]; }
    inline const_iterator cend()            const noexcept { return &memory_ptr()[size()]; }

    inline reverse_iterator rbegin()        const noexcept { return std::reverse_iterator<iterator>(end()); }
    inline const_reverse_iterator crbegin() const noexcept { return std::reverse_iterator<const_iterator>(end()); }
    inline reverse_iterator rend()          const noexcept { return std::reverse_iterator<iterator>(begin()); }
    inline const_reverse_iterator crend()   const noexcept { return std::reverse_iterator<const_iterator>(begin()); }

    inline reference       operator[](size_type pos)       { return memory_ptr()[pos]; }
    inline const_reference operator[](size_type pos) const { return memory_ptr()[pos]; }

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

    inline void push_back(const value_type& value) {
        emplace_back(value);
    }

    inline void push_back(value_type&& value) {
        emplace_back(std::forward<T>(value));
    }

    template<class... Args>
    reference emplace_back(Args&&... args) {
        if (capacity() < (sizeof(T) * (size() + 1)) ) {
            memory_.resize();
        }
        T* ptr = &memory_ptr()[count_];
        new (ptr) T;
        *ptr = T(std::forward<Args>(args)...);
        count_ += 1;
        return *ptr;
    }

 private:
  inline T* memory_ptr() const noexcept { return reinterpret_cast<T*>(memory_.pointer()); }

  Memory memory_;
  std::size_t count_ = 0;
};
