#include <iostream>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

#include "virtual_vec.h"

#ifdef TEST
#include <gtest/gtest.h>
#endif  // #ifdef TEST

#ifdef BENCH
#include <benchmark/benchmark.h>
#endif  // #ifdef BENCH

#if !defined TEST && !defined BENCH
int main() {
    std::cout << "hello wordl!";
    std::vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    std::cout << "start our implementation" << std::endl;
    for (size_t i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl << "end!" << std::endl;

    virtual_vec<std::string> tester;
    tester.emplace_back("Test");
    tester.emplace_back(std::string("ing"));
    //std::string r("real");
    //tester.emplace_back(std::move(r));
    //auto x = tester.data();
    //std::cout << std::addressof(x) << std::endl;

    for (size_t i = 0; i < tester.size(); i++) {
         std::cout << tester[i] << std::endl;
    }

    std::string* m4 = (std::string*) malloc(sizeof(std::string) * 300);
    std::string x("test");
    new (m4) std::string;
    m4[0] = std::move(x);

    std::cout << "test:" << m4[0] << std::endl;

}

#endif // #ifndef TEST

#ifdef TEST

TEST(VirtualVectorTest, TestSizeSimple) {
    virtual_vec<int> v;
    v.push_back(1);
    EXPECT_EQ(1, v.size());
    v.push_back(2);
    EXPECT_EQ(2, v.size());
    v.push_back(3);
    EXPECT_EQ(3, v.size());
}

TEST(VirtualVectorTest, TestIndexSimple) {
    virtual_vec<int> v;
    for (int i = 0; i < 5; i++) {
        v.push_back(i);
    }
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(i, v[i]);
    }
}

TEST(VirtualVectorTest, TestAtSimple) {
    virtual_vec<int> v;
    v.push_back(1);
    EXPECT_EQ(1, v.at(0));
}

TEST(VirtualVectorTest, TestAtThrows) {
    virtual_vec<int> v;
    EXPECT_THROW(v.at(1000), std::out_of_range);
}

TEST(VirtualVectorTest, TestEmplaceBack) {
    virtual_vec<std::string> v;
    v.emplace_back("Hello");
    std::string s("cruel");
    v.emplace_back(std::move(s));
    v.emplace_back(std::string("world"));
    EXPECT_TRUE("Hello" == v[0]);
    EXPECT_TRUE("cruel" == v[1]);
    EXPECT_TRUE("world" == v[2]);
}

TEST(VirtualVectorTest, TestLargePushBack) {
    int elems = (1 << 20) / sizeof(int64_t); // ~roughly 1MB

    virtual_vec<int> myvec;
    std::vector<int> stlvec;

    for (int64_t i = 0; i < elems; i++) {
        int next_num = rand();
        myvec.push_back(next_num);
        stlvec.push_back(next_num);
    }

    ASSERT_EQ(stlvec.size(), myvec.size());
    for (std::vector<int>::size_type i = 0; i < stlvec.size(); i++) {
        ASSERT_EQ(stlvec[i], myvec[i]);
    }
}

TEST(VirtualVectorTest, TestForwardIterator) {
    virtual_vec<int> v;
    for (int i = 0; i < 5; i++) {
        v.push_back(i);
    }
    int called_count = 0;
    for (auto it = v.begin(); it != v.end(); it++, called_count++) {
        EXPECT_EQ(*it, std::distance(v.begin(), it));
    }
    ASSERT_EQ(v.size(), called_count);
}

TEST(VirtualVectorTest, TestReverseIterator) {
    virtual_vec<int> v;
    for (int i = 0; i < 5; i++) {
        v.push_back(i);
    }
    int called_count = 0;
    for (auto it = v.rbegin(); it != v.rend(); it++, called_count++) {
        EXPECT_EQ(*it,  std::distance(it, v.rend()) - 1);
    }
    ASSERT_EQ(v.size(), called_count);
}

#endif  // #ifdef TEST

#ifdef BENCH

template <template<typename T> class VectorType>
static void BV_vector(benchmark::State& state) {
    VectorType<int64_t> v;
    for (auto _ : state) {
        int64_t count = state.range(0) / sizeof(int64_t);
        for (int64_t i = 0; i < count; i++) {
            v.push_back(i);
        }
    }
}

BENCHMARK_TEMPLATE1(BV_vector, std::vector)->RangeMultiplier(2)->Range(10, 10 << 20);
BENCHMARK_TEMPLATE1(BV_vector, virtual_vec)->RangeMultiplier(2)->Range(10, 10 << 20);

#endif  // #ifdef BENCH