#include "HttpInfo.hpp"
#include "Proxy.hpp"

using namespace nlohmann;
using namespace geode::prelude;
using namespace proxy::prelude;
using namespace asp::time;
using namespace web;

static std::atomic_size_t globalIndexCounter = 1;

const LookupTable<ContentType, converters::Converter*> HttpInfo::CONVERTERS({
    { ContentType::BINARY, new RawToBinary() },
    { ContentType::XML, new XmlToXml() },
    { ContentType::JSON, new JsonToJson() },
    { ContentType::ROBTOP, new RobTopToJsonV2() },
    { ContentType::FORM, new FormToJson() }
});

HttpInfo::Content HttpInfo::getContent(const bool raw, const ContentType originalContentType, const std::string_view path, const std::string_view original, const bool censor) {
    if (HttpInfo::CONVERTERS.contains(originalContentType)) {
        const converters::Converter* converter = HttpInfo::CONVERTERS.at(originalContentType);

        if (!raw) {
            return { converter->getResultContentType(), converter->convert(path, original, censor) };
        } else if (censor && converter->isNeedsCensoring() && Mod::get()->getSettingValue<bool>("censor-data")) {
            return { originalContentType, converter->toRaw(path, converter->convert(path, original, censor)) };
        }
    }

    // TODO: This is copying, see if there's any way around it
    return { originalContentType, UniqueString(original) };
}

HttpInfo::Content HttpInfo::getHeaders(const bool raw, const Headers& headers) {
    if (raw) {
        std::string rawHeaders;

        for (const auto& [headerKey, list] : headers) {
            for (const std::string_view headerValue : list) {
                rawHeaders.append(headerKey)
                    .append(": ")
                    .append(headerValue)
                    .push_back('\n');
            }
        }

        return { ContentType::UNKNOWN_CONTENT, std::move(rawHeaders) };
    } else {
        ordered_json content = ordered_json::object();

        for (const auto& [headerKey, list] : headers) {
            content[headerKey] = list.size() == 1 ? json(list.at(0)) : json(list);
        }

        return { ContentType::JSON, converters::safeDump(content) };
    }
}

ContentType HttpInfo::determineContentType(const std::string_view path, const bool isRequest, const std::string_view body) {
    if (body.empty()) return ContentType::UNKNOWN_CONTENT;

    for (const auto& [type, converter] : HttpInfo::CONVERTERS) {
        if (converter->canConvert(path, isRequest, body)) {
            return type;
        }
    }

    return ContentType::UNKNOWN_CONTENT;
}

HttpInfo::Headers HttpInfo::parseCocosHeaders(const gd::vector<char>* headers) {
    return HttpInfo::parseCocosHeaders(StringUtils::split(std::string_view(headers->data(), headers->size()), '\n')
        .map([](std::string&& header) {
            if (header.back() == '\r') {
                header.pop_back();
            }

            return gd::string(std::move(header));
        })
        .filter([](const std::string_view header) { return header.size(); }));
}

HttpInfo::Headers HttpInfo::parseCocosHeaders(const gd::vector<gd::string>& headers) {
    Headers parsed;

    for (const std::string_view header : headers) {
        const size_t colonPos = header.find(':');

        if (colonPos != std::string_view::npos) {
            const size_t valueOffset = colonPos + 1;
            const size_t valueStartOffset = header.find_first_not_of(' ', valueOffset);

            parsed[std::string(header.substr(0, colonPos))].emplace_back(header.substr(
                valueStartOffset == std::string_view::npos ? valueOffset : valueStartOffset
            ));
        } else {
            parsed["<GDI_CUSTOM:Keyless>"].emplace_back(header);
        }
    }

    return parsed;
}

HttpInfo::Headers HttpInfo::parseHeaderListStrings(Headers headers) {
    for (auto& [key, values] : headers) {
        headers.at(key) = Stream(values)
            .map([](std::string&& value) { return StringUtils::split(std::move(value), ';'); })
            .flat()
            .map([](std::string&& value) { return StringUtils::trim(std::move(value)); });
    }

    return headers;
}

HttpInfo::HttpInfo(CCHttpRequest* request) : m_id(globalIndexCounter++),
m_client(Client::COCOS),
m_state(Mod::get()->getSavedValue<bool>("paused") ? State::PAUSED : State::IN_PROGRESS),
m_repeat(false),
m_url(request->getUrl()),
m_request(request, this) { }

HttpInfo::HttpInfo(const WebRequest& request) : m_id(globalIndexCounter++),
m_client(Client::GEODE),
m_state(State::IN_PROGRESS),
m_repeat(request.getMod() == Mod::get()),
m_url(request.getUrl(), request),
m_request(request, this) { }

void HttpInfo::cancel() {
    if (m_state == State::IN_PROGRESS || m_state == State::PAUSED) {
        m_state = State::CANCELLED;

        ProxyEvent(EventState::CANCEL).send(shared_from_this());
    }
}

bool HttpInfo::isPaused() const {
    return m_state == State::PAUSED;
}

bool HttpInfo::isInProgress() const {
    return m_state == State::IN_PROGRESS;
}

bool HttpInfo::isCompleted() const {
    return m_state == State::COMPLETED;
}

bool HttpInfo::isFaulty() const {
    return m_state == State::FAULTY;
}

bool HttpInfo::isCancelled() const {
    return m_state == State::CANCELLED;
}

bool HttpInfo::hasResponse() const {
    return m_response.has_value();
}

std::string HttpInfo::getHttpVersion() const {
    return std::visit(makeVisitor{
        [](CCHttpRequest* request) {
            return "HTTP/1.1";
        },
        [](const web::WebRequest& request) {
            switch (request.getHttpVersion()) {
                case HttpVersion::VERSION_1_0: return "HTTP/1.0";
                case HttpVersion::VERSION_1_1: return "HTTP/1.1";
                case HttpVersion::VERSION_2_0: return "HTTP/2";
                case HttpVersion::VERSION_3: return "HTTP/3";
                default: return "HTTP/2";
            }
        }
    }, m_request.m_original);
}

std::string HttpInfo::toString(const bool cutContent) const {
    Duration requestTime = m_request.m_requestTime.timeSinceEpoch();
    Content bodyContent = m_request.getBodyContent();
    std::string scheme = m_url.getStringScheme();
    utils::StringBuffer info;

    std::transform(scheme.begin(), scheme.end(), scheme.begin(), toupper);

    std::visit(makeVisitor{
        [&info](CCHttpRequest* request) {
            info.append("Client: Cocos2D-X");
        },
        [&info](const web::WebRequest& request) {
            info.append("Client: Geode\nMod ID: {}", request.getMod()->getID());
        }
    }, m_request.m_original);

    info.append("\nRequest Time: {}.{}\nHTTP Version: {}\nMethod: {}\nScheme: {}\nHost: {}",
        geode::localtime(requestTime.seconds()),
        requestTime.millis() % 1000,
        this->getHttpVersion(),
        m_request.getMethod(),
        scheme,
        m_url.getHost()
    );

    if (m_url.getUsername().size() || m_url.getPassword().size()) {
        info.append("\nUsername: {}\nPassword: {}", m_url.getUsername(), m_url.getPassword());
    }

    info.append("\nPath: {}", m_url.getPath());

    if (!cutContent) {
        info.append("\nQuery: {}\nRequest Headers: {}\nBody:", m_url.stringifyQuery(), m_request.getHeaderContent(false).contents.view());

        if (bodyContent.contents.size()) {
            info.append(bodyContent.type == ContentType::JSON ? ' ' : '\n');
            info.append(bodyContent.contents.view());
        }
    }

    info.append("\nBody Size: {} Bytes", m_request.getBodySize());

    if (m_url.getHash().size()) {
        info.append("\nHash: {}", m_url.getHash());
    }

    if (m_state == State::CANCELLED) {
        info.append("\nStatus Code: Request Cancelled");
    }

    if (!this->hasResponse()) {
        return info.str();
    }

    info.append("\nStatus Code: {}\nResponse Time: {}", m_response->stringifyStatusCode(), m_response->getResponseTime());

    if (!cutContent) {
        const Content bodyContent = m_response->getBodyContent();

        info.append("\nResponse Headers: {}\nResponse:", m_response->getBodyContent(false).contents.view());

        if (bodyContent.contents.size()) {
            info.append(bodyContent.type == ContentType::JSON ? ' ' : '\n');
            info.append(bodyContent.contents.view());
        }
    }

    info.append("\nResponse Size: {} Bytes", m_response->getBodySize());

    return info.str();
}

std::string HttpInfo::toCurl() const {
    StringBuffer command;

    command.append("curl -X {} {}", m_request.getMethod(), m_url.getReconstruction());

    for (const auto& [key, values] : m_request.getHeaders()) {
        command.append(" -H \"{}: {}\"",
            StringUtils::replace(key, "\"", "\\\""),
            StringUtils::replace(StringUtils::join(values, "; "), "\"", "\\\"")
        );
    }

    if (m_request.getBodySize()) {
        command.append(" -d \"{}\"", StringUtils::replace(m_request.getBody(), "\"", "\\\""));
    }

    return command.str();
}

void HttpInfo::resume() {
    m_state = State::IN_PROGRESS;
}

/////////////////////////////////////////////////////////////////////
// REQUEST
/////////////////////////////////////////////////////////////////////

HttpInfo::Request::Request(OriginalRequest original, HttpInfo* info) : m_original(std::move(original)),
m_info(info),
m_contentType(HttpInfo::determineContentType(info->m_url.getPath(), true, this->getBody())) {
    this->resetTime();

    if (std::holds_alternative<CCHttpRequest*>(m_original)) {
        std::get<CCHttpRequest*>(m_original)->retain();
    }
}

HttpInfo::Request::~Request() {
    if (std::holds_alternative<CCHttpRequest*>(m_original)) {
        std::get<CCHttpRequest*>(m_original)->release();
    }
}

std::string HttpInfo::Request::getMethod() const {
    return std::visit<std::string>(makeVisitor{
        [](CCHttpRequest* request) {
            switch (request->getRequestType()) {
                case CCHttpRequest::HttpRequestType::kHttpGet: return "GET";
                case CCHttpRequest::HttpRequestType::kHttpPost: return "POST";
                case CCHttpRequest::HttpRequestType::kHttpPut: return "PUT";
                case CCHttpRequest::HttpRequestType::kHttpDelete: return "DELETE";
                default: return "UNKNOWN";
            }
        },
        [](const web::WebRequest& request) { return request.getMethod(); }
    }, m_original);
}

HttpInfo::Headers HttpInfo::Request::getHeaders() const {
    return HttpInfo::parseHeaderListStrings(std::visit(makeVisitor{
        [](CCHttpRequest* request) { return HttpInfo::parseCocosHeaders(request->getHeaders()); },
        [](const web::WebRequest& request) { return request.getHeaders(); }
    }, m_original));
}

std::string_view HttpInfo::Request::getBody() const {
    return std::visit(makeVisitor{
        [](CCHttpRequest* request) { return std::string_view(request->getRequestData(), request->getRequestDataSize()); },
        [](const web::WebRequest& request) {
            // TODO: Make this .data()
            const std::optional<ByteVector>& body = request.getBody();

            return body.has_value() ? std::string_view(reinterpret_cast<const char*>(body->data()), body->size()) : std::string_view();
        }
    }, m_original);
}

size_t HttpInfo::Request::getBodySize() const {
    return std::visit<size_t>(makeVisitor{
        [](CCHttpRequest* request) { return request->getRequestDataSize(); },
        [](const web::WebRequest& request) {
            // TODO: Make this .data()
            return request.getBody()->size();
        }
    }, m_original);
}

HttpInfo::Content HttpInfo::Request::getHeaderContent(const bool raw) const {
    return HttpInfo::getHeaders(raw, this->getHeaders());
}

HttpInfo::Content HttpInfo::Request::getBodyContent(const bool raw, const bool censor) const {
    return HttpInfo::getContent(raw, m_contentType, m_info->m_url.getPath(), this->getBody(), censor);
}

void HttpInfo::Request::resetTime() {
    m_requestTime = SystemTime::now();
}

/////////////////////////////////////////////////////////////////////
// RESPONSE
/////////////////////////////////////////////////////////////////////

HttpInfo::Response::Response(OriginalResponse original, HttpInfo* info) : m_original(std::move(original)),
m_info(info),
m_responseTime((SystemTime::now() - info->m_request.m_requestTime).value_or(Duration())),
m_contentType(HttpInfo::determineContentType(info->m_url.getPath(), false, this->getBody())) {
    if (std::holds_alternative<CCHttpResponse*>(m_original)) {
        // TODO: Use geode::Ref
        std::get<CCHttpResponse*>(m_original)->retain();
    }
}

HttpInfo::Response::~Response() {
    if (std::holds_alternative<CCHttpResponse*>(m_original)) {
        std::get<CCHttpResponse*>(m_original)->release();
    }
}

int HttpInfo::Response::getStatusCode() const {
    return std::visit(makeVisitor{
        [](CCHttpResponse* response) { return response->getResponseCode(); },
        [](const web::WebResponse& response) { return response.code(); }
    }, m_original);
}

HttpInfo::Headers HttpInfo::Response::getHeaders() const {
    return HttpInfo::parseHeaderListStrings(std::visit(makeVisitor{
        [](CCHttpResponse* response) { return HttpInfo::parseCocosHeaders(response->getResponseHeader()); },
        [](const web::WebResponse& response) {
            Headers headers;

            for (std::string header : response.headers()) {
                std::vector<std::string> value = response.getAllHeadersNamed(header).value_or(std::vector<std::string>());

                headers[std::move(header)] = std::move(value);
            }

            return HttpInfo::parseHeaderListStrings(std::move(headers));
        }
    }, m_original));
}

std::string_view HttpInfo::Response::getBody() const {
    return std::visit(makeVisitor{
        [](CCHttpResponse* response) {
            gd::vector<char>* responseData = response->getResponseData();

            return std::string_view(responseData->data(), responseData->size());
        },
        [](const web::WebResponse& response) {
            const ByteVector& responseData = response.data();

            return std::string_view(reinterpret_cast<const char*>(responseData.data()), responseData.size());
        }
    }, m_original);
}

size_t HttpInfo::Response::getBodySize() const {
    return std::visit(makeVisitor{
        [](CCHttpResponse* response) { return response->getResponseData()->size(); },
        [](const web::WebResponse& response) { return response.data().size(); }
    }, m_original);
}

std::string HttpInfo::Response::stringifyStatusCode() const {
    const int statusCode = this->getStatusCode();

    switch (statusCode) {
        case static_cast<int>(GeodeWebError::CURL_INITIALIZATION_ERROR): return "Curl Initialization Fail";
        case static_cast<int>(GeodeWebError::REQUEST_CANCELLED): return "Request Cancelled";
        case static_cast<int>(GeodeWebError::QUEUE_FULL): return "Async Queue Full";
        case static_cast<int>(GeodeWebError::CHANNEL_CLOSED): return "Channel Closed";
        default: return utils::numToString(statusCode);
    }
}

HttpInfo::Content HttpInfo::Response::getHeaderContent(const bool raw) const {
    return HttpInfo::getHeaders(raw, this->getHeaders());
}

HttpInfo::Content HttpInfo::Response::getBodyContent(const bool raw, const bool censor) const {
    return HttpInfo::getContent(raw, m_contentType, m_info->m_url.getPath(), this->getBody(), censor);
}