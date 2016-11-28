#include <gtest/gtest.h>

#include <tudocomp/compressors/lzss/LZSSCoding.hpp>
#include <tudocomp/compressors/lzss/LZSSFactors.hpp>
#include <tudocomp/compressors/lzss/LZSSLiterals.hpp>

using namespace tdc;

TEST(lzss, factor_buffer_empty) {
    lzss::FactorBuffer buf;

    // empty buffer
    ASSERT_TRUE(buf.empty());
    ASSERT_EQ(0, buf.size());
    ASSERT_TRUE(buf.is_sorted());
}

TEST(lzss, factor_buffer_sorted) {
    // sorted buffer
    const size_t n = 10;
    lzss::FactorBuffer buf;

    for(size_t i = 0; i < n; i++) {
        buf.push_back(lzss::Factor(i, i + n, n + 1 - i));
    }

    ASSERT_FALSE(buf.empty());
    ASSERT_EQ(n, buf.size());
    ASSERT_TRUE(buf.is_sorted());
}

TEST(lzss, factor_buffer_sort) {
    // unsorted buffer
    const size_t n = 10;
    lzss::FactorBuffer buf;

    for(size_t i = n; i > 0; i--) {
        buf.push_back(lzss::Factor(n + i, 2 * n + i, 2 * n - i));
    }

    ASSERT_FALSE(buf.is_sorted());

    buf.sort();
    ASSERT_TRUE(buf.is_sorted());

    for(size_t i = 0; i < buf.size() - 1; i++) {
        ASSERT_LE(buf[i].pos, buf[i+1].pos);
    }
}

TEST(lzss, text_literals_empty) {
    lzss::FactorBuffer empty;
    lzss::TextLiterals<std::string> literals("", empty);
    ASSERT_FALSE(literals.has_next());
}

template<typename text_t>
void lzss_text_literals_factors(
    lzss::TextLiterals<text_t>& literals,
    const std::string& ref_literals,
    const len_t* ref_positions) {

    size_t i = 0;
    while(literals.has_next()) {
        auto l = literals.next();
        ASSERT_EQ(ref_literals[i],  l.c);
        ASSERT_EQ(ref_positions[i], l.pos);
        ++i;
    }

    ASSERT_EQ(ref_literals.length(), i);
}

TEST(lzss, text_literals_nofactors) {
    std::string text = "abcdefgh";
    const len_t positions[] = {0,1,2,3,4,5,6,7};

    lzss::FactorBuffer empty;
    lzss::TextLiterals<std::string> literals(text, empty);

    lzss_text_literals_factors(literals, text, positions);
}

TEST(lzss, text_literals_factors_middle) {
    std::string text = "a__b____cd___e";
    std::string ref_literals = "abcde";
    const len_t ref_positions[] = {0,3,8,9,13};

    lzss::FactorBuffer factors;
    factors.push_back(lzss::Factor(1, text.length(), 2));
    factors.push_back(lzss::Factor(4, text.length(), 4));
    factors.push_back(lzss::Factor(10, text.length(), 3));

    lzss::TextLiterals<std::string> literals(text, factors);

    lzss_text_literals_factors(literals, ref_literals, ref_positions);
}

TEST(lzss, text_literals_factors_begin) {
    std::string text = "___a__bc__de";
    std::string ref_literals = "abcde";
    const len_t ref_positions[] = {3,6,7,10,11};

    lzss::FactorBuffer factors;
    factors.push_back(lzss::Factor(0, text.length(), 3));
    factors.push_back(lzss::Factor(4, text.length(), 2));
    factors.push_back(lzss::Factor(8, text.length(), 2));

    lzss::TextLiterals<std::string> literals(text, factors);

    lzss_text_literals_factors(literals, ref_literals, ref_positions);
}

TEST(lzss, text_literals_factors_end) {
    std::string text = "a___b__cd__e__";
    std::string ref_literals = "abcde";
    const len_t ref_positions[] = {0,4,7,8,11};

    lzss::FactorBuffer factors;
    factors.push_back(lzss::Factor(1, text.length(), 3));
    factors.push_back(lzss::Factor(5, text.length(), 2));
    factors.push_back(lzss::Factor(9, text.length(), 2));
    factors.push_back(lzss::Factor(12, text.length(), 2));

    lzss::TextLiterals<std::string> literals(text, factors);

    lzss_text_literals_factors(literals, ref_literals, ref_positions);
}