#ifndef UTIL_H_
#define UTIL_H_

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string_view>

static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";

template <size_t MaxLen> struct FixedString {
    char data[MaxLen + 1]{};
    size_t len = 0;

    consteval void push(char c) {
        data[len++] = c;
        data[len] = '\0';
    }

    constexpr const char *c_str() const { return data; }
    constexpr operator std::string_view() const { return {data, len}; }
};

static consteval bool is_url_safe(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' ||
           c == '.' || c == '_' || c == '~';
}

/// @brief URL-encode a string literal at compile time.
/// @tparam N The size of the input string literal (including null terminator).
/// @param input The input string literal.
/// @return A FixedString containing the URL-encoded result.
template <size_t N> consteval auto url_encode(const char (&input)[N]) {
    FixedString<3 * (N - 1)> result;

    for (size_t i = 0; i < N - 1; ++i) {
        auto c = static_cast<unsigned char>(input[i]);
        if (is_url_safe(static_cast<char>(c))) {
            result.push(static_cast<char>(c));
        } else {
            result.push('%');
            result.push(HEX_DIGITS[(c >> 4) & 0xF]);
            result.push(HEX_DIGITS[c & 0xF]);
        }
    }

    return result;
}

/// @brief Concatenate a string literal and a FixedString at compile time.
template <size_t N, size_t M> consteval auto concat(const char (&a)[N], const FixedString<M> &b) {
    FixedString<(N - 1) + M> result;
    for (size_t i = 0; i < N - 1; ++i)
        result.push(a[i]);
    for (size_t i = 0; i < b.len; ++i)
        result.push(b.data[i]);
    return result;
}

/// @brief Attempt to parse a string into a number.
/// @tparam TNumber The numeric type to parse into.
/// @param input The input string.
/// @param out A reference to a number to assign the result to, if parsing is successful.
/// @return A boolean indicating whether parsing succeeded.
template <typename TNumber>
static bool try_parse_number(const std::string_view &input, TNumber &out) {
    const std::from_chars_result result =
        std::from_chars(input.data(), input.data() + input.size(), out);

    if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
        return false;
    }

    return true;
}

#endif // UTIL_H_