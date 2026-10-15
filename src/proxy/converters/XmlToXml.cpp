#include "XmlToXml.hpp"

proxy::converters::XmlToXml::XmlToXml() : Converter(enums::ContentType::XML) { }

bool proxy::converters::XmlToXml::canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const {
    return original.starts_with("<?xml") ||
        original.starts_with("<!DOCTYPE") ||
        original.starts_with("<html");
}

std::string proxy::converters::XmlToXml::convert(const std::string_view path, const std::string_view original, const bool censor) const {
    return std::string(original);
}

std::string proxy::converters::XmlToXml::toRaw(const std::string_view path, const std::string_view original) const {
    return std::string(original);
}
