#include "URL.hpp"

using namespace nlohmann;
using namespace proxy::enums;
using namespace geode::prelude;

proxy::URL::URL(std::string url) : m_original(std::move(url)), m_reconstruction(m_original), m_query(ordered_json::object()) {
    this->parse();
}

proxy::URL::URL(std::string url, const web::WebRequest& request) : m_original(std::move(url)), m_reconstruction(m_original), m_query(ordered_json::object()) {
    const StringMap<std::string> params = request.getUrlParams();

    if (params.size() && m_reconstruction.find('?') == std::string::npos) {
        m_reconstruction.push_back('?');
    }

    for (const auto& [key, value] : params) {
        m_reconstruction.append(key);
        m_reconstruction.push_back('=');
        m_reconstruction.append(value);
        m_reconstruction.push_back('&');
    }

    if (m_reconstruction.ends_with('&')) {
        m_reconstruction.pop_back();
    }

    this->parse();
}

std::string proxy::URL::getBasicUrl() const {
    return fmt::format("{}://{}/{}", m_stringScheme, this->getHost(), m_path);
}

std::string proxy::URL::stringifyQuery(const bool raw) const {
    return converters::safeDump(m_query, raw ? -1 : 2);
}

void proxy::URL::parse() {
    size_t index = 0;

    this->parseScheme(index);
    this->parseLogin(index);

    if (m_scheme != Scheme::FILE && m_scheme != Scheme::STEAM && m_scheme != Scheme::MAIL && m_scheme != Scheme::UNKNOWN_SCHEME) {
        this->parseHost(index);
        this->determineOrigin();
    } else {
        m_origin = Origin::OTHER;
    }

    this->parsePath(index);
}

void proxy::URL::parseScheme(size_t& index) {
    static const geode::utils::StringMap<Scheme> translations {
        {"http", Scheme::HTTP},
        {"https", Scheme::HTTPS},
        {"ws", Scheme::WS},
        {"wss", Scheme::WSS},
        {"ftp", Scheme::FTP},
        {"sftp", Scheme::SFTP},
        {"file", Scheme::FILE},
        {"steam", Scheme::STEAM},
        {"mailto", Scheme::MAIL},
    };
    const size_t schemeEnd = m_reconstruction.find(":", index);
    const size_t size = schemeEnd - index;

    if (schemeEnd == std::string::npos || (
        m_stringScheme = m_reconstruction.substr(index, size)
    ).find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789") != std::string::npos) {
        m_scheme = Scheme::UNKNOWN_SCHEME;
        m_stringScheme = "unknown";

        return;
    } else {
        m_scheme = translations.contains(m_stringScheme) ? translations.at(m_stringScheme) : Scheme::UNKNOWN_SCHEME;
    }

    std::transform(m_stringScheme.begin(), m_stringScheme.end(), m_stringScheme.begin(), tolower);

    index += size + 1 + m_reconstruction.substr(schemeEnd + 1).starts_with("//") * 2;
}

void proxy::URL::parseLogin(size_t& index) {
    const size_t loginEnd = m_reconstruction.find('@', index);
    const size_t size = loginEnd - index;
    const std::string login = m_reconstruction.substr(index, size);

    if (loginEnd != std::string::npos && login.find('/') == std::string::npos) {
        const size_t split = login.find(':');

        m_username = login.substr(0, split);

        if (split != std::string::npos) {
            m_password = login.substr(split + 1);
        }

        index += size + 1;
    }
}

void proxy::URL::parseHost(size_t& index) {
    const size_t hostEnd = m_reconstruction.find('/', index);
    const size_t size = (hostEnd == std::string::npos ? m_reconstruction.size() : hostEnd) - index;

    m_subDomains = StringUtils::split(m_host = m_reconstruction.substr(index, size), '.');

    std::string& lastSubDomain = m_subDomains.back();
    const size_t portStart = lastSubDomain.find(':');

    switch (m_scheme) {
        case Scheme::WSS: [[fallthrough]];
        case Scheme::HTTPS: {
            m_port = 443;
        } break;
        case Scheme::FTP: {
            m_port = 21;
        } break;
        case Scheme::SFTP: {
            m_port = 22;
        } break;
        default: {
            m_port = 80;
        }
    }

    if (portStart != std::string::npos) {
        m_port = utils::numFromString<unsigned int>(lastSubDomain.substr(portStart + 1)).unwrapOr(m_port);
        lastSubDomain = lastSubDomain.substr(0, portStart);
    }

    if (lastSubDomain == "localhost") {
        m_domain = m_domainName = lastSubDomain;
        m_subDomains.pop_back();
    } else for (size_t i = m_subDomains.size() - 1; i >= 0; i--) {
        std::string subDomain = m_subDomains[i];

        m_subDomains.pop_back();

        if (m_tld.empty()) {
            m_domain = '.' + subDomain;
            m_tld = std::move(subDomain);
        } else {
            m_domain = subDomain + m_domain;
            m_domainName = std::move(subDomain);

            break;
        }
    }

    index += size;
}

void proxy::URL::parsePath(size_t& index) {
    converters::FormToJson converter;
    std::string fullPath = m_reconstruction.substr(index);
    const size_t size = fullPath.size();
    const size_t queryStart = fullPath.find('?');
    const size_t hashStart = fullPath.find('#');
    const size_t pathEnd = queryStart == std::string::npos ? hashStart == std::string::npos ? size : hashStart : queryStart;

    m_path = pathEnd == 0 ? "/" : fullPath.substr(0, pathEnd);

    if (hashStart != std::string::npos) {
        m_hash = fullPath.substr(hashStart + 1);
        fullPath = fullPath.substr(0, hashStart);
    }

    if (queryStart != std::string::npos && converter.canConvert(m_path, false, m_queryString = fullPath.substr(queryStart + 1))) {
        m_query = ordered_json::parse(converter.convert(m_path, m_queryString, true));
    }

    index += size;
}

void proxy::URL::determineOrigin() {
    if (m_domain == "boomlings.com" || m_domain == "geometrydash.com") {
        m_origin = Origin::GD;
    } else if (m_domain == "geometrydashfiles.b-cdn.net" || m_domain == "geometrydashcontent.b-cdn.net") {
        m_origin = Origin::GD_CDN;
    } else if (
        m_domain == "robtopgames.org" ||
        m_domain == "robtopgames.net" ||
        m_domain == "robtopgames.com"
    ) {
        m_origin = Origin::ROBTOP_GAMES;
    } else if (m_domain == "ngfiles.com") {
        m_origin = Origin::NEWGROUNDS;
    } else if (m_domain == "geode-sdk.org") {
        m_origin = Origin::GEODE;
    } else if (
        m_domain == "localhost" ||
        m_domain.starts_with("127.")
    ) {
        m_origin = Origin::LOCALHOST;
    } else if (
        m_domain.starts_with("0.") ||
        m_domain.starts_with("10.") ||
        m_domain.starts_with("192.168.")
    ) {
        m_origin = Origin::LAN;
    } else {
        m_origin = Origin::OTHER;
    }
}