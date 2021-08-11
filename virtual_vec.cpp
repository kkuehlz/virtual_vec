#include "virtual_vec.h"

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <stdexcept>
#include <sys/mman.h>

#include <iostream>

namespace {
    constexpr size_t ADDRESS_SPACE = (8ULL << 30);
    constexpr size_t PAGE_SIZE = (4ULL << 10);

    template <typename T>
    inline T page_align(T p) {
        return PAGE_SIZE + (p & (-PAGE_SIZE));
    }
};

Memory::Memory() {
    void* memory = mmap(nullptr, ADDRESS_SPACE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
       throw std::runtime_error("Could not allocate");
       return;
    }
    memory_ = static_cast<uint8_t*>(memory);
    num_bytes_ = 0;
    resize(1);
}

void Memory::resize(size_t wanted) {
    std::ptrdiff_t len = page_align(wanted);
#if 0
    std::cout << "resizing:" << wanted << " " << len << std::endl;;
#endif
    num_bytes_ = len;
    int r = mprotect(memory_, len, PROT_READ | PROT_WRITE);
    if (r != 0) {
        throw std::runtime_error("Could not mprotect");
        return;
    }
}