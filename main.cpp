#include "virtual_vec.h"

#include <vector>

#ifdef TEST
#include <gtest/gtest.h>
#endif  // #ifdef TEST

#ifdef BENCH
#include <benchmark/benchmark.h>
#endif  // #ifdef BENCH

#ifdef TEST

// For testing nontrivial classes
template <class... Args>
static inline std::string make_non_sso_string(Args&&... args) {
    static_assert(std::is_trivial<std::string>::value == false);
    std::string s("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + std::forward<std::string>(args)...);
    return s;
}

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

TEST(VirtualVectorTest, TestInitializerList) {
    virtual_vec<int> v{1, 2, 3};
    ASSERT_EQ(3, v.size());
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(i + 1, v[i]);
    }

    v = {0, 2, 4, 6, 8, 10, 12, 14};
    ASSERT_EQ(8, v.size());
    for (int i =0; i < 8; i++) {
        ASSERT_EQ(i * 2, v[i]);
    }
}

TEST(VirtualVectorTest, TestCopy) {
    virtual_vec<std::string> v0{make_non_sso_string("one"), make_non_sso_string("two")};
    virtual_vec<std::string> v1(v0);
    ASSERT_EQ(v0.size(), v1.size());
    virtual_vec<std::string> v2 = v1;
    ASSERT_EQ(v1.size(), v2.size());
    for (size_t i = 0; i < v0.size(); i++) {
        ASSERT_EQ(v1[i], v2[i]);
    }
}

TEST(VirtualVectorTest, TestMove) {
    virtual_vec<std::string> v0{make_non_sso_string("one")};
    virtual_vec<std::string> v1(std::move(v0));
    EXPECT_TRUE(v0.size() == 0);
    ASSERT_TRUE(v1.size() == 1);
    ASSERT_TRUE(v1[0] == make_non_sso_string("one"));
    virtual_vec<std::string> v3{make_non_sso_string("three")};
    v0 = std::move(v3);
    EXPECT_TRUE(v3.size() == 0);
    ASSERT_TRUE(v0.size() == 1);
    ASSERT_TRUE(v0[0] == make_non_sso_string("three"));
}

TEST(VirtualVectorTest, TestFillConstructor) {
    std::string t(make_non_sso_string("test"));
    virtual_vec<std::string> v(10, t);
    ASSERT_EQ(10, v.size());
    for (const auto& s : v) {
        EXPECT_EQ(t, s);
    }
}

TEST(VirtualVectorTest, TestEmplace) {
    virtual_vec<std::string> v{
        make_non_sso_string("one"),
        make_non_sso_string("two"),
        make_non_sso_string("three"),
        make_non_sso_string("five"),
        make_non_sso_string("six")
    };
    virtual_vec<std::string> expected = v;
    auto new_it = v.emplace(v.begin() + 3, make_non_sso_string("four"));
    ASSERT_EQ(6, v.size());
    ASSERT_EQ(*new_it, make_non_sso_string("four"));
    auto it = v.begin();
    for (const auto& s : { make_non_sso_string("one"), make_non_sso_string("two"), make_non_sso_string("three"), make_non_sso_string("four"), make_non_sso_string("five"), make_non_sso_string("six")} ){
        EXPECT_EQ(*it++, s);
    }
}

TEST(VirtualVectorTest, TestEmplaceBounds) {
    virtual_vec<int> v{1};
    auto begin_it = v.emplace(v.begin(), 0);
    ASSERT_EQ(*begin_it, 0);
    auto end_it = v.emplace(v.end(), 2);
    ASSERT_EQ(*end_it, 2);
    ASSERT_EQ(3, v.size());
    for (size_t i = 0; i < v.size(); i++) {
        EXPECT_EQ(i, v[i]);
    }
}

TEST(VirtualVectorTest, TestEmplaceEmpty) {
    virtual_vec<std::string> v0;
    auto v0_it = v0.emplace(v0.begin(), make_non_sso_string("v0"));
    ASSERT_EQ(*v0_it, make_non_sso_string("v0"));
    ASSERT_EQ(1, v0.size());

    virtual_vec<std::string> v1;
    auto v1_it = v1.emplace(v1.end(), make_non_sso_string("v1"));
    ASSERT_EQ(*v1_it, make_non_sso_string("v1"));
    ASSERT_EQ(1, v1.size());
}

TEST(VirtualVectorTest, TestEmplaceBack) {
    virtual_vec<std::string> v;
    v.emplace_back(make_non_sso_string("Hello"));
    std::string s = make_non_sso_string("cruel");
    v.emplace_back(std::move(s));
    v.emplace_back(make_non_sso_string("world"));
    EXPECT_TRUE(make_non_sso_string("Hello") == v[0]);
    EXPECT_TRUE(make_non_sso_string("cruel") == v[1]);
    EXPECT_TRUE(make_non_sso_string("world") == v[2]);
}

TEST(VirtualVectorTest, TestInsertList) {
    std::vector<int> input{1, 2, 3};
    virtual_vec<int> output{0, 4};
    output.insert(output.begin() + 1, input.begin(), input.end());
    ASSERT_EQ(5, output.size());
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(i, output[i]);
    }
    ASSERT_EQ(3, input.size());
}

TEST(VirtualVectorTest, TestInsertFill) {
  virtual_vec<std::string> v{make_non_sso_string("start"), make_non_sso_string("end")};
  std::string filler = make_non_sso_string("filler");
  v.insert(v.begin() + 1, 5, filler);
  ASSERT_EQ(7, v.size());
  EXPECT_EQ(make_non_sso_string("start"), v.front());
  EXPECT_EQ(make_non_sso_string("end"), v.back());
  for (size_t i = 1; i < 6; i++) {
      EXPECT_EQ(filler, v[i]);
  }
}

TEST(VirtualVectorTest, TestInsertListEmpty) {
    virtual_vec<std::string> v0, v1;
    v0.insert(v0.begin(), { make_non_sso_string("hello") });
    ASSERT_EQ(1, v0.size());
    ASSERT_EQ(make_non_sso_string("hello"), v0[0]);

    std::vector<std::string> sample {make_non_sso_string("world")};
    v1.insert(v1.end(), sample.begin(), sample.end());
    ASSERT_EQ(1, v1.size());
    ASSERT_EQ(make_non_sso_string("world"), sample[0]);
    ASSERT_EQ(make_non_sso_string("world"), v1[0]);
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

TEST(VirtualVectorTest, TestEraseOnPrimitive) {
    virtual_vec<int> myvec;
    std::vector<int> stlvec;
    for (int i = 0; i < 20; i++) {
        myvec.push_back(i);
        stlvec.push_back(i);
    }

    auto erase0 = myvec.erase(myvec.begin() + 5);
    auto erase1 = stlvec.erase(stlvec.begin() + 5);
    ASSERT_EQ(stlvec.size(), myvec.size());
    ASSERT_EQ(*erase0, *erase1);

    for (size_t i = 0; i < myvec.size(); i++) {
        ASSERT_EQ(stlvec[i], myvec[i]);
    }
}

TEST(VirtualVectorTest, TestEraseNontrivial) {
    virtual_vec<std::string> myvec;
    std::vector<std::string> stlvec;
    for (int i = 0; i < 20; i++) {
        myvec.emplace_back(make_non_sso_string(std::to_string(i)));
        stlvec.emplace_back(make_non_sso_string(std::to_string(i)));
    }

    auto erase0 = myvec.erase(myvec.begin() + 1, myvec.begin() + 6);
    auto erase1 = stlvec.erase(stlvec.begin() + 1, stlvec.begin() + 6);
    ASSERT_EQ(stlvec.size(), myvec.size());
    ASSERT_EQ(*erase0, *erase1);

    for (size_t i = 0; i < myvec.size(); i++) {
        ASSERT_EQ(stlvec[i], myvec[i]);
    }
}

TEST(VirtualVectorTest, EnsureShrinkOccurs) {
    virtual_vec<int> v;
    v.reserve(10000);
    v.push_back(1);
    auto old_cap = v.capacity();
    v.shrink_to_fit();
    auto new_cap = v.capacity();
    EXPECT_TRUE(new_cap < old_cap);
}

TEST(VirtualVectorTest, TestSwap) {
   virtual_vec<int> v0{0, 1, 2}, v1{3, 4, 5};
   v0.swap(v1);
   for (int i = 0; i < 3; i++) {
       ASSERT_EQ(i, v1[i]);
       ASSERT_EQ(i + 3,  v0[i]);
   }
}

TEST(VirtualVectorTest, TestResizeMore) {
    virtual_vec<std::string> v{make_non_sso_string("one")};
    std::string filler = make_non_sso_string("two");
    v.resize(3, filler);
    ASSERT_EQ(v.size(), 3);
    ASSERT_EQ(make_non_sso_string("one"), v[0]);
    for (size_t i = 1; i < v.size(); i++) {
        EXPECT_EQ(filler, v[i]);
    }
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
