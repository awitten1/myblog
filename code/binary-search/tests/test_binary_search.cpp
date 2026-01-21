#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "functions.hpp"

namespace {

void ExpectMatchesLowerBound(const std::vector<long>& data, long target) {
    auto expected = static_cast<size_t>(
        std::lower_bound(data.begin(), data.end(), target) - data.begin());
    EXPECT_EQ(binary_search(data, target), expected);
    EXPECT_EQ(branchless_binary_search(data, target), expected);
    EXPECT_EQ(linear_search(data, target), expected);
}

}  // namespace

TEST(BinarySearch, EmptyVector) {
    std::vector<long> data;
    ExpectMatchesLowerBound(data, 0);
    ExpectMatchesLowerBound(data, 42);
}

TEST(BinarySearch, SingleElement) {
    std::vector<long> data = {5};
    ExpectMatchesLowerBound(data, 3);
    ExpectMatchesLowerBound(data, 5);
    ExpectMatchesLowerBound(data, 7);
}

TEST(BinarySearch, SortedDistinctValues) {
    std::vector<long> data = {1, 3, 5, 7, 9};
    ExpectMatchesLowerBound(data, 0);
    ExpectMatchesLowerBound(data, 1);
    ExpectMatchesLowerBound(data, 2);
    ExpectMatchesLowerBound(data, 5);
    ExpectMatchesLowerBound(data, 10);
}

TEST(BinarySearch, Duplicates) {
    std::vector<long> data = {1, 2, 2, 2, 3};
    ExpectMatchesLowerBound(data, 2);
    ExpectMatchesLowerBound(data, 4);
}

TEST(BinarySearch, LargerSortedRange) {
    std::vector<long> data;
    data.reserve(1000);
    for (long i = 0; i < 1000; ++i) {
        data.push_back(i * 2);
    }
    ExpectMatchesLowerBound(data, -1);
    ExpectMatchesLowerBound(data, 0);
    ExpectMatchesLowerBound(data, 777);
    ExpectMatchesLowerBound(data, 1999);
    ExpectMatchesLowerBound(data, 2001);
}

TEST(BinarySearch, AllEqualValues) {
    std::vector<long> data(128, 5);
    ExpectMatchesLowerBound(data, 4);
    ExpectMatchesLowerBound(data, 5);
    ExpectMatchesLowerBound(data, 6);
}

TEST(BinarySearch, RandomizedData) {
    std::mt19937 gen(12345);
    std::uniform_int_distribution<long> dist(-1000, 1000);

    std::vector<long> data(512);
    for (auto& value : data) {
        value = dist(gen);
    }
    std::sort(data.begin(), data.end());

    for (int i = 0; i < 50; ++i) {
        ExpectMatchesLowerBound(data, dist(gen));
    }
}
