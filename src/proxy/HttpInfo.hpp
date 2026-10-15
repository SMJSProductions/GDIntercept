#pragma once

#include "../include.hpp"
#include "converters/Converter.hpp"
#include "converters/XmlToXml.hpp"
#include "converters/FormToJson.hpp"
#include "converters/JsonToJson.hpp"
#include "converters/RawToBinary.hpp"
#include "converters/RobTopToJsonV2.hpp"
#include "URL.hpp"

namespace proxy {
    class HttpInfo : std::enable_shared_from_this<HttpInfo> {
        friend class ProxyHandler;
    public:
        using Headers = geode::utils::StringMap<std::vector<std::string>>;

        struct Content {
            enums::ContentType type;
            UniqueString contents;
        };

        class Request {
            using OriginalRequest = std::variant<CCHttpRequest*, web::WebRequest>;

            friend class ProxyHandler;
            friend class HttpInfo;

            GETTER(OriginalRequest, original, Original);
            GETTER(asp::SystemTime, requestTime, RequestTime);
            enums::ContentType m_contentType;
            HttpInfo* m_info;
        public:
            Request& operator=(const Request&) = delete;

            Request(const Request&) = delete;
            Request(OriginalRequest original, HttpInfo* info);
            ~Request();
            std::string getMethod() const;
            Headers getHeaders() const;
            std::string_view getBody() const;
            size_t getBodySize() const;
            Content getHeaderContent(const bool raw = false) const;
            Content getBodyContent(const bool raw = true, const bool censor = true) const;
        private:
            void resetTime();
        };

        class Response {
            using OriginalResponse = std::variant<CCHttpResponse*, web::WebResponse>;

            friend class ProxyHandler;
            friend class HttpInfo;

            GETTER(OriginalResponse, original, Original);
            GETTER(asp::Duration, responseTime, ResponseTime);
            enums::ContentType m_contentType;
            HttpInfo* m_info;
        public:
            Response& operator=(const Response&) = delete;

            Response(const Response&) = delete;
            Response(HttpInfo* info);
            Response(OriginalResponse original, HttpInfo* info);
            ~Response();
            int getStatusCode() const;
            Headers getHeaders() const;
            std::string_view getBody() const;
            size_t getBodySize() const;
            std::string stringifyStatusCode() const;
            Content getHeaderContent(const bool raw = false) const;
            Content getBodyContent(const bool raw = true, const bool censor = true) const;
        };
    private:
        static const LookupTable<enums::ContentType, converters::Converter*> CONVERTERS;

        static Content getContent(const bool raw, const enums::ContentType originalContentType, const std::string_view path, const std::string_view original, const bool censor);
        static Content getHeaders(const bool raw, const Headers& headers);
        static enums::ContentType determineContentType(const std::string_view path, const bool isRequest, const std::string_view body);
        static Headers parseCocosHeaders(const gd::vector<char>* headers);
        static Headers parseCocosHeaders(const gd::vector<gd::string>& headers);
        static Headers parseHeaderListStrings(Headers headers);

        PRIMITIVE_GETTER(size_t, id, ID);
        PRIMITIVE_GETTER(enums::Client, client, Client);
        PRIMITIVE_GETTER(enums::State, state, State);
        GETTER(URL, url, URL);
        GETTER(Request, request, Request);
        GETTER(std::optional<Response>, response, Response);
        BOOL_GETTER(repeat, Repeat);
    public:
        HttpInfo& operator=(const HttpInfo&) = delete;

        HttpInfo(const HttpInfo&) = delete;
        HttpInfo(cocos2d::extension::CCHttpRequest* request);
        HttpInfo(const geode::utils::web::WebRequest& request);
        void cancel();
        bool isPaused() const;
        bool isInProgress() const;
        bool isCompleted() const;
        bool isFaulty() const;
        bool isCancelled() const;
        bool hasResponse() const;
        std::string getHttpVersion() const;
        std::string toString(const bool cutContent = false) const;
        std::string toCurl() const;
    private:
        void resume();
    };
}