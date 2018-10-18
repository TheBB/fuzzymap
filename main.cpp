#define CATCH_CONFIG_MAIN

#include "test/catch.hpp"
#include "fuzzymap.h"


template<typename T>
void check_hash(T value, int prec) {
    typedef typename fuzzymap<T, int>::hash_t hash_t;
    hash_t abin = fabs(value) / pow(2, prec);
    hash_t bin = value < 0.0 ? abin | FloatDigits<T>::sign_mask : abin;

    typename fuzzymap<T, int>::hash_t l, b;
    hashes(value, &l, &b, prec);

    REQUIRE(b == bin);

    if (abin == 0)
        REQUIRE(l == (bin ^ FloatDigits<T>::sign_mask));
    else
        REQUIRE(l == bin - 1);
}

template<typename T>
void check_hashes(int prec, int num) {
    T scale = pow(2, prec);
    for (int k = -num; k < num; k++)
        check_hash((k + (T)0.5) * scale, prec);
}


TEST_CASE("Correct hashes (double, prec=1)", "[hash]") {
    check_hashes<double>(1, 100);
}

TEST_CASE("Correct hashes (double, prec=0)", "[hash]") {
    check_hashes<double>(0, 100);
}

TEST_CASE("Correct hashes (double, prec=-1)", "[hash]") {
    check_hashes<double>(-1, 100);
}

TEST_CASE("Correct hashes (double, prec=-2)", "[hash]") {
    check_hashes<double>(-2, 100);
}

TEST_CASE("Correct hashes (double, prec=-3)", "[hash]") {
    check_hashes<double>(-3, 200);
}

TEST_CASE("Correct hashes (double, prec=-4)", "[hash]") {
    check_hashes<double>(-4, 400);
}

TEST_CASE("Correct hashes (float, prec=1)", "[hash]") {
    check_hashes<float>(1, 100);
}

TEST_CASE("Correct hashes (float, prec=0)", "[hash]") {
    check_hashes<float>(0, 100);
}

TEST_CASE("Correct hashes (float, prec=-1)", "[hash]") {
    check_hashes<float>(-1, 100);
}

TEST_CASE("Correct hashes (float, prec=-2)", "[hash]") {
    check_hashes<float>(-2, 100);
}

TEST_CASE("Correct hashes (float, prec=-3)", "[hash]") {
    check_hashes<float>(-3, 200);
}

TEST_CASE("Correct hashes (float, prec=-4)", "[hash]") {
    check_hashes<float>(-4, 400);
}

TEST_CASE("Lookup", "[map]") {
    fuzzymap<double, int> map(0.25);

    map[0.1] = 1;
    map[0.6] = 2;
    REQUIRE(map[0.1] == 1);
    REQUIRE(map[0.2] == 1);
    REQUIRE(map[0.6] == 2);
    REQUIRE(map[0.7] == 2);
}
