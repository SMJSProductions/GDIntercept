#include "Converter.hpp"

using namespace nlohmann;
using namespace geode::prelude;

proxy::converters::Converter::Converter(const enums::ContentType resultContentType, const bool needsMasking) : m_needsCensoring(resultContentType == enums::ContentType::JSON),
m_resultContentType(resultContentType),
m_needsMasking(needsMasking) { }

bool proxy::converters::isInt(const std::string_view str) {
    if (str.empty()) {
        return false;
    }

    const bool negative = str.at(0) == '-';

    return str.substr(negative, str.size() - negative).find_first_not_of("0123456789") == std::string::npos && (!negative || str.size() > 1);
}

bool proxy::converters::isNumber(const std::string_view str) {
    if (str.empty()) {
        return false;
    }

    const size_t decimalPos = str.find('.');

    return converters::isInt(str.substr(0, decimalPos)) && (
        decimalPos == std::string_view::npos ||
        str.find_first_not_of("0123456789", decimalPos + 1) == std::string_view::npos
    );
}

bool proxy::converters::isBool(const std::string_view str) {
    return str == "true" || str == "false";
}

bool proxy::converters::isNull(const std::string_view str) {
    return str == "null";
}

bool proxy::converters::isString(const std::string_view str) {
    if (str.size() < 2) {
        return false;
    }

    const char firstChar = str.at(0);

    if ((firstChar != '"' && firstChar != '\'') || str.back() != firstChar) {
        return false;
    } else if (str.size() == 2) {
        return true;
    }

    bool isEscaped = false;

    for (size_t i = 1; i < str.size() - 1; i++) {
        if (isEscaped) {
            isEscaped = false;
        } else if (str[i] == firstChar) {
            return false;
        } else if (str[i] == '\\') {
            isEscaped = true;
        }
    }

    return !isEscaped;
}

bool proxy::converters::shouldCensor(const std::string_view key) {
    static const std::vector<std::string> SENSITIVE_KEYS {
        "password",
        "pass",
        "passwd",
        "pwd",
        "token",
        "tkn",
        "sID",
        "gjp",
        "gjp2"
    };

    return Mod::get()->getSettingValue<bool>("censor-data") &&  std::find(SENSITIVE_KEYS.begin(), SENSITIVE_KEYS.end(), key) != SENSITIVE_KEYS.end();
}

std::string proxy::converters::safeDump(const nlohmann::ordered_json& json, const size_t indent, const bool quoteless) {
    if (quoteless && json.is_string()) {
        return json.get<std::string>();
    } else {
        return json.dump(indent, ' ', true, json::error_handler_t::replace);
    }
}

nlohmann::json proxy::converters::getPrimitiveJsonType(const std::string_view key, const std::string_view str, const bool censor) {
    if (censor && converters::shouldCensor(key)) {
        return json("********");
    } else if (converters::isNull(str)) {
        return json();
    } else if (converters::isBool(str)) {
        return json(str == "true");
    } else if (converters::isInt(str)) {
        if (Result<long long> result = utils::numFromString<long long>(str); result.isOk()) {
            return json(result.unwrap());
        } else {
            return json(str);
        }
    } else if (converters::isNumber(str)) {
        if (Result<long double> result = utils::numFromString<long double>(str); result.isOk()) {
            return json(result.unwrap());
        } else {
            return json(str);
        }
    } else if (converters::isString(str)) {
        return json(str.substr(1, str.size() - 2));
    } else {
        return json(str);
    }
}