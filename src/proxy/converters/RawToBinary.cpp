#include "RawToBinary.hpp"

proxy::converters::RawToBinary::RawToBinary() : Converter(enums::ContentType::BINARY) { };

bool proxy::converters::RawToBinary::canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const {
    // Anything beyond 5Mb risks being too much to handle
    if (original.size() > 5'000'000) return true;

    for (const u_char c : original) {
        // Anything under space is a non text char which must be considered binary
        if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D) return true;
    }

    return false;
}

std::string proxy::converters::RawToBinary::convert(const std::string_view path, const std::string_view original, const bool censor) const {
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string result;

    if (original.empty()) {
        return result;
    }

    result.reserve(original.size() * 3 - 1);

    for (size_t i = 0; i < original.size(); i++) {
        const unsigned char byte = static_cast<unsigned char>(original[i]);

        if (i != 0) {
            result.push_back((i % 16 == 0) ? '\n' : ' ');
        }

        result.push_back(hex[byte >> 4]);
        result.push_back(hex[byte & 0xF]);
    }

    return result;
}

std::string proxy::converters::RawToBinary::toRaw(const std::string_view path, const std::string_view original) const {
    std::string result;

    result.reserve(original.size() / 3 + 1);

    for (size_t i = 0; (i + 1) < original.size(); i += 3) {
        result.push_back((this->hexValue(original[i]) << 4) | this->hexValue(original[i + 1]));
    }

    return result;
}