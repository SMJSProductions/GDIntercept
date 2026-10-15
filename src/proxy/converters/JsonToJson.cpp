#include "JsonToJson.hpp"

using namespace nlohmann;

proxy::converters::JsonToJson::JsonToJson() : Converter(enums::ContentType::JSON) { };

bool proxy::converters::JsonToJson::canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const {
    return json::accept(original);
}

std::string proxy::converters::JsonToJson::convert(const std::string_view path, const std::string_view original, const bool censor) const {
    ordered_json object = ordered_json::parse(original);

    if (censor) {
        recursiveCensor(object);
    }

    return converters::safeDump(object);
}

std::string proxy::converters::JsonToJson::toRaw(const std::string_view path, const std::string_view original) const {
    return converters::safeDump(json::parse(original), -1);
}

void proxy::converters::JsonToJson::recursiveCensor(ordered_json& object) const {
    for (const auto& [key, value] : object.items()) {
        if (value.is_array() || value.is_object()) {
            recursiveCensor(value);
        } else if (converters::shouldCensor(key)) {
            object[key] = json("********");
        }
    }
}