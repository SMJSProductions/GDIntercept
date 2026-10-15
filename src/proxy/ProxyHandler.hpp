#pragma once

#include <shared_mutex>
#include "../include.hpp"
#include "Proxy.hpp"

namespace proxy {
    class ProxyHandler : private CCObject {
    public:
        static ProxyHandler* create(CCHttpRequest* request);
        static ProxyHandler* create(web::WebRequest& request);
        static Stream<ProxyHandler*> getFilteredProxies();
        static bool isProxy(CCHttpRequest* request);
        static bool isProxy(const web::WebRequest& request);
        static bool isPaused();
        static void pauseAll();
        static void resumeAll();
        static void setCacheLimit(const size_t limit);

        ProxyHandler& operator=(const ProxyHandler&) = delete;

        ProxyHandler(CCHttpRequest* request);
        ProxyHandler(web::WebRequest& request);
        ProxyHandler(const ProxyHandler&) = delete;

        std::shared_ptr<HttpInfo> getInfo() noexcept;
    private:
        static Stream<ProxyHandler*> ALIVE_PROXIES;
        static std::shared_mutex ALIVE_MUTEX;
        static std::vector<ProxyHandler*> PAUSED_PROXIES;
        static std::mutex PAUSED_MUTEX;

        static void registerProxy(ProxyHandler* proxy);

        std::shared_ptr<HttpInfo> m_info;
        std::optional<ListenerHandle> m_responseListener;
        std::vector<ListenerHandle> m_proxyListeners;
        CCObject* m_originalTarget;
        SEL_HttpResponse m_originalListener;
        bool m_finished;

        void onCocosResponse(CCHttpClient* client, CCHttpResponse* response);
        bool onModResponse(const web::WebResponse& response);
        void onResponse();
    };
}