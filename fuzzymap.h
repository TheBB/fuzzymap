#include <cfloat>
#include <climits>
#include <cmath>
#include <limits>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

#include <iostream>

template<typename K>
struct FloatDigits;

template<>
struct FloatDigits<float> {
    typedef uint32_t hash_t;
    static constexpr uint64_t const bias = 127;

    // Subtract one because one of the digits is not stored
    static constexpr int8_t const mantissa_digits = FLT_MANT_DIG - 1;
    static constexpr hash_t const mantissa_mask = std::numeric_limits<hash_t>::max() >> (33 - FLT_MANT_DIG);
    static constexpr hash_t const exponent_mask = (std::numeric_limits<hash_t>::max() >> 1) & (~mantissa_mask);
    static constexpr hash_t const sign_mask = static_cast<hash_t>(1) << 31;
};

template<>
struct FloatDigits<double> {
    typedef uint64_t hash_t;
    static constexpr uint64_t const bias = 1023;

    // Subtract one because one of the digits is not stored
    static constexpr int8_t const mantissa_digits = DBL_MANT_DIG - 1;
    static constexpr hash_t const mantissa_mask = std::numeric_limits<hash_t>::max() >> (65 - DBL_MANT_DIG);
    static constexpr hash_t const exponent_mask = (std::numeric_limits<hash_t>::max() >> 1) & (~mantissa_mask);
    static constexpr hash_t const sign_mask = static_cast<hash_t>(1) << 63;
};


template<typename K, typename H>
void hashes(K _value, H* lbin, H* bin, int precision) {
    typedef FloatDigits<K> spec;

    H value = *reinterpret_cast<H*>(&_value);

    H exp = (spec::exponent_mask & value) >> spec::mantissa_digits;
    if (exp < precision + spec::bias)
        *lbin = *bin = 0;
    else {
        H z = exp - precision - spec::bias;
        H mantissa_block = spec::mantissa_mask & value;
        mantissa_block >>= spec::mantissa_digits - z;
        *bin = (1LL << z) + mantissa_block;
        *lbin = *bin - 1;
    }

    if (spec::sign_mask & value) {
        if (*bin > 0)
            *lbin ^= spec::sign_mask;
        *bin ^= spec::sign_mask;
    }
    else if (*bin == 0)
        *lbin ^= spec::sign_mask;
}


template<typename K, typename V>
class fuzzymap {
    static_assert(std::is_floating_point<K>::value, "Key must be a floating point type");
    typedef FloatDigits<K> spec;

public:
    typedef typename spec::hash_t hash_t;

private:
    typedef std::pair<K, V> pair;
    typedef std::map<hash_t, std::vector<pair>> submap;
    typedef typename submap::iterator subiter;

    V* _lookup(hash_t hash, K key, bool add = false) {
        subiter it = map.find(hash);
        if (it != map.end())
            for (pair& p : it->second)
                if (abs(p.first - key) < gap)
                    return &p.second;
        if (add) {
            map[hash].push_back(pair(key, 0.0));
            return &map[hash].back().second;
        }
        return nullptr;
    }

public:
    fuzzymap<K, V>(K gap) : gap(gap) {
        precision = (int64_t) ceil(log2(gap));
    }

    V& operator[](const K& key) {
        hash_t lbin, bin;
        hashes(key, &lbin, &bin, precision);

        V* ptr;
        if (ptr = _lookup(bin, key)) return *ptr;
        if (ptr = _lookup(lbin, key)) return *ptr;
        if (ptr = _lookup(bin + 1, key)) return *ptr;
        return *_lookup(bin, key, true);
    }

private:
    submap map;
    int64_t precision;
    K gap;
};
