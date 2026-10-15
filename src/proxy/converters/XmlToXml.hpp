#pragma once

#include "Converter.hpp"

namespace proxy::converters {
    class XmlToXml : public Converter {
    public:
        XmlToXml();
        bool canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const override;
        std::string convert(const std::string_view path, const std::string_view original, const bool censor) const override;
        std::string toRaw(const std::string_view path, const std::string_view original) const override;
    };
}