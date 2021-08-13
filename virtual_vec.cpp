#include "virtual_vec.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <sys/mman.h>

#ifndef _NO_QUERY_PAGE_SIZE
#include <unistd.h>
#endif  // #ifndef _NO_QUERY_PAGE_SIZE

namespace {

#ifndef _NO_QUERY_PAGE_SIZE
    class OSPageSizeCache {
    public:
        static OSPageSizeCache& Get() {
            static OSPageSizeCache instance;
            return instance;
        }

        inline size_t GetPageSize() const noexcept {
            return page_size_;
        }

    private:
        OSPageSizeCache(const OSPageSizeCache&) = delete;
        OSPageSizeCache& operator=(const OSPageSizeCache) = delete;
        OSPageSizeCache() {
            long page_size = sysconf(_SC_PAGESIZE);
            if (page_size < 0) {
                throw std::runtime_error("Could not sysconf");
            } else {
                page_size_ = static_cast<size_t>(page_size);
            }
        }

        size_t page_size_;
    };
#endif  // #ifndef _NO_QUERY_PAGE_SIZE

    inline size_t GetPageSize() {
#ifdef _NO_QUERY_PAGE_SIZE
        constexpr size_t DEFAULT_PAGE_SIZE = (4ULL << 10);
        return DEFAULT_PAGE_SIZE;
#else
        return OSPageSizeCache::Get().GetPageSize();
#endif
    }

    template <typename T>
    inline T page_align(T p, T page_size = GetPageSize()) {
        return (p + ( page_size - 1) ) & ~( (page_size - 1) );
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
        throw std::runtime_error("Could not reserve");
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