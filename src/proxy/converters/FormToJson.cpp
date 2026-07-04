#include "FormToJson.hpp"

using namespace nlohmann;

proxy::converters::FormToJson::FormToJson() : Converter(enums::ContentType::JSON, true) { };

bool proxy::converters::FormToJson::canConvert(const std::string_view path, const bool isBody, const std::string_view original) const {
    bool hasAssignment = false;

    for (const char character : original) {
        if (character == '\n') {
            return false;
        } else if (character == '=') {
            hasAssignment = true;
        } else if (character == '&') {
            if (!hasAssignment) {
                return false;
            }

            hasAssignment = false;
        }
    }

    return hasAssignment;
}

std::string proxy::converters::FormToJson::convert(const std::string_view path, const std::string_view original, const bool censor) const {
    ordered_json object(ordered_json::object());

    StringUtils::split(original, '&').forEach([&object, censor](const std::string_view section) {
        const size_t equalPos = section.find('=');
        const std::string_view key = section.substr(0, equalPos);

        if (equalPos == std::string::npos) {
            object[key] = json("");
        } else {
            object[key] = getPrimitiveJsonType(key, section.substr(equalPos + 1), censor);
        }
    });

    return converters::safeDump(object);
}

std::string proxy::converters::FormToJson::toRaw(const std::string_view path, const std::string_view original) const {
    const ordered_json object = ordered_json::parse(original);
    std::string result;

    for (const auto& [key, value] : object.items()) {
        result.append(key);
        result.push_back('=');
        result.append(converters::safeDump(value, 0, true));
        result.push_back('&');
    }

    result.pop_back();

    return result;
}