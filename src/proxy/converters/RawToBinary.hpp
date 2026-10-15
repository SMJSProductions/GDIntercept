#pragma once

#include "Converter.hpp"

namespace proxy::converters {
    // What can I say? I like it raw
    class RawToBinary : public Converter {
    public:
        RawToBinary();
        bool canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const override;
        std::string convert(const std::string_view path, const std::string_view original, const bool censor) const override;
        std::string toRaw(const std::string_view path, const std::string_view original) const override;
    private:
        inline unsigned char hexValue(const unsigned char character) const {
            if (character >= '0' && character <= '9') {
                return character - '0';
            } else if (character >= 'A' && character <= 'F') {
                return character - 'A' + 10;
            } else {
                return 0;
            }
        }
    };
}