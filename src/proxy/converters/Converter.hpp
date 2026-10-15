#pragma once

#include "../../include.hpp"
#include "../Enums.hpp"

namespace proxy::converters {
    bool isInt(const std::string_view str);
    bool isNumber(const std::string_view str);
    bool isBool(const std::string_view str);
    bool isNull(const std::string_view str);
    bool isString(const std::string_view str);
    bool shouldCensor(const std::string_view key);
    std::string safeDump(const nlohmann::ordered_json& json, const size_t indent = 2, const bool quoteless = false);
    nlohmann::json getPrimitiveJsonType(const std::string_view key, const std::string_view str, const bool censor);

    class Converter {
    public:
        Converter(const enums::ContentType resultContentType, const bool needsMasking = false);
        virtual bool canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const = 0;
        virtual std::string convert(const std::string_view path, const std::string_view original, const bool censor) const = 0;
        virtual std::string toRaw(const std::string_view path, const std::string_view original) const = 0;
        BOOL_PROTECTED_GETTER(needsCensoring, NeedsCensoring);
        PRIMITIVE_GETTER(enums::ContentType, resultContentType, ResultContentType);
    };
}