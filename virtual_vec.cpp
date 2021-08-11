#include "virtual_vec.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <sys/mman.h>

namespace {
    constexpr size_t PAGE_SIZE = (4ULL << 10);

    template <typename T>
    inline T page_align(T p) {
        return (p + ( PAGE_SIZE - 1) ) & ~( (PAGE_SIZE - 1) );
    }
};

Memory::~Memory() {
    if (memory_) {
        munmap(memory_, num_bytes());
    }
}

void Memory::reserve() {
    void* memory = mmap(nullptr, Memory::avail_mem(), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        throw std::runtime_error("Could not allocate");
        return;
    }
    memory_ = static_cast<uint8_t*>(memory);
    num_bytes_ = 0;
}

void Memory::grow(size_t wanted) {
    if (memory_ == nullptr) { reserve(); }

    std::ptrdiff_t len = page_align(wanted);
    num_bytes_ = len;
    int r = mprotect(memory_, len, PROT_READ | PROT_WRITE);
    if (r != 0) {
        throw std::runtime_error("Could not mprotect");
    }
}

void Memory::shrink(size_t wanted) {
    if (memory_ == nullptr) { return; }

    std::ptrdiff_t len = page_align(wanted);
    auto remaining = num_bytes() - len;
    if (remaining == 0) { return; }
    if (remaining < 0) {
        remaining = len;
        len = 0;
    }
    num_bytes_ = len;
    int r = mprotect(memory_ + len, remaining, PROT_NONE);
    if (r != 0) {
        throw std::runtime_error("Could not mprotect");
    }
}