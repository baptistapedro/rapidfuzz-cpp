/* SPDX-License-Identifier: MIT */
/* Copyright © 2020 Max Bachmann */

#include <algorithm>
#include <array>
#include <cctype>
#include <cwctype>

namespace rapidfuzz {

template <typename CharT1, typename CharT2>
bool string_view_eq(basic_string_view<CharT1> x, basic_string_view<CharT2> y)
{
    if (x.size() != y.size()) return false;

    for (std::size_t i = 0; i < x.size(); ++i) {
        if (x[i] != y[i]) return false;
    }
    return true;
}

template <typename CharT1, typename CharT2>
DecomposedSet<CharT1, CharT2, CharT1> common::set_decomposition(SplittedSentenceView<CharT1> a,
                                                                SplittedSentenceView<CharT2> b)
{
    a.dedupe();
    b.dedupe();

    string_view_vec<CharT1> intersection;
    string_view_vec<CharT1> difference_ab;
    string_view_vec<CharT2> difference_ba = b.words();

    for (const auto& current_a : a.words()) {
        auto element_b = std::find_if(difference_ba.begin(), difference_ba.end(),
                                      [current_a](basic_string_view<CharT2> current_b) {
                                          return string_view_eq(current_a, current_b);
                                      });

        if (element_b != difference_ba.end()) {
            difference_ba.erase(element_b);
            intersection.push_back(current_a);
        }
        else {
            difference_ab.push_back(current_a);
        }
    }

    return {difference_ab, difference_ba, intersection};
}

template <typename Sentence, typename CharT, typename>
basic_string_view<CharT> common::to_string_view(Sentence&& str)
{
    return basic_string_view<CharT>(std::forward<Sentence>(str));
}

template <typename Sentence, typename CharT, typename>
basic_string_view<CharT> common::to_string_view(const Sentence& str)
{
    return basic_string_view<CharT>(str.data(), str.size());
}

template <typename Sentence, typename CharT, typename>
std::basic_string<CharT> common::to_string(Sentence&& str)
{
    return std::basic_string<CharT>(std::forward<Sentence>(str));
}

template <typename Sentence, typename CharT, typename>
std::basic_string<CharT> common::to_string(const Sentence& str)
{
    return std::basic_string<CharT>(str.data(), str.size());
}

template <typename InputIterator1, typename InputIterator2>
std::pair<InputIterator1, InputIterator2>
common::mismatch(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
                 InputIterator2 last2)
{
    while (first1 != last1 && first2 != last2 && *first1 == *first2) {
        ++first1;
        ++first2;
    }
    return std::pair<InputIterator1, InputIterator2>(first1, first2);
}

/**
 * Removes common prefix of two string views
 */
template <typename CharT1, typename CharT2>
std::size_t common::remove_common_prefix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b)
{
    std::size_t prefix = static_cast<std::size_t>(
        std::distance(a.begin(), common::mismatch(a.begin(), a.end(), b.begin(), b.end()).first));
    a.remove_prefix(prefix);
    b.remove_prefix(prefix);
    return prefix;
}

/**
 * Removes common suffix of two string views
 */
template <typename CharT1, typename CharT2>
std::size_t common::remove_common_suffix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b)
{
    std::size_t suffix = static_cast<std::size_t>(std::distance(
        a.rbegin(), common::mismatch(a.rbegin(), a.rend(), b.rbegin(), b.rend()).first));
    a.remove_suffix(suffix);
    b.remove_suffix(suffix);
    return suffix;
}

/**
 * Removes common affix of two string views
 */
template <typename CharT1, typename CharT2>
StringAffix common::remove_common_affix(basic_string_view<CharT1>& a, basic_string_view<CharT2>& b)
{
    return StringAffix{remove_common_prefix(a, b), remove_common_suffix(a, b)};
}

template <typename, typename = void>
struct is_space_dispatch_tag : std::integral_constant<int, 0> {};

template <typename CharT>
struct is_space_dispatch_tag<CharT, typename std::enable_if<sizeof(CharT) == 1>::type>
    : std::integral_constant<int, 1> {};

/*
 * Implementation of is_space for char types that are at least 2 Byte in size
 */
template <typename CharT>
bool is_space_impl(const CharT ch, std::integral_constant<int, 0>)
{
    switch (ch) {
    case 0x0009:
    case 0x000A:
    case 0x000B:
    case 0x000C:
    case 0x000D:
    case 0x001C:
    case 0x001D:
    case 0x001E:
    case 0x001F:
    case 0x0020:
    case 0x0085:
    case 0x00A0:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200A:
    case 0x2028:
    case 0x2029:
    case 0x202F:
    case 0x205F:
    case 0x3000:
        return true;
    }
    return false;
}

/*
 * Implementation of is_space for char types that are 1 Byte in size
 */
template <typename CharT>
bool is_space_impl(const CharT ch, std::integral_constant<int, 1>)
{
    switch (ch) {
    case 0x0009:
    case 0x000A:
    case 0x000B:
    case 0x000C:
    case 0x000D:
    case 0x001C:
    case 0x001D:
    case 0x001E:
    case 0x001F:
    case 0x0020:
        return true;
    }
    return false;
}

/*
 * checks whether unicode characters have the bidirectional
 * type 'WS', 'B' or 'S' or the category 'Zs'
 */
template <typename CharT>
bool is_space(const CharT ch)
{
    return is_space_impl(ch, is_space_dispatch_tag<CharT>{});
}

template <typename Sentence, typename CharT>
SplittedSentenceView<CharT> common::sorted_split(Sentence&& sentence)
{
    auto s = to_string_view(std::forward<Sentence>(sentence));
    string_view_vec<CharT> splitted;
    const auto* first = s.data();
    const auto* second = s.data();
    const auto* last = first + s.size();

    for (; second != last && first != last; first = second + 1) {
        second = std::find_if(first, last, is_space<CharT>);

        if (first != second) {
            splitted.emplace_back(first, second - first);
        }
    }

    std::sort(splitted.begin(), splitted.end());

    return SplittedSentenceView<CharT>(splitted);
}

} // namespace rapidfuzz
