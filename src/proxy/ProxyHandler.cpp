#include "ProxyHandler.hpp"

using namespace proxy::prelude;

$on_mod(Loaded) {
    Mod::get()->setSavedValue("paused", Mod::get()->getSettingValue<bool>("pause-between-plays") && Mod::get()->getSavedValue("paused", false));

    web::WebRequestInterceptEvent().listen([](const std::string_view modID, web::WebRequest& request) {
        ProxyHandler::create(request);

        return ListenerResult::Propagate;
    }, 1'000'000).leak();
}

Stream<ProxyHandler*> proxy::ProxyHandler::ALIVE_PROXIES;

std::shared_mutex proxy::ProxyHandler::ALIVE_MUTEX;

std::vector<ProxyHandler*> proxy::ProxyHandler::PAUSED_PROXIES;

std::mutex proxy::ProxyHandler::PAUSED_MUTEX;

ProxyHandler* ProxyHandler::create(CCHttpRequest* request) {
    ProxyHandler* proxy = new ProxyHandler(request);

    ProxyHandler::registerProxy(proxy);

    return proxy;
}

ProxyHandler* ProxyHandler::create(web::WebRequest& request) {
    ProxyHandler* proxy = new ProxyHandler(request);

    ProxyHandler::registerProxy(proxy);

    return proxy;
}

Stream<ProxyHandler*> ProxyHandler::getFilteredProxies() {
    std::shared_lock aliveLock(ProxyHandler::ALIVE_MUTEX);
    std::string filter = Mod::get()->getSettingValue<std::string>("filter");

    if (ProxyHandler::ALIVE_PROXIES.empty() || filter == "None") {
        return Stream(ProxyHandler::ALIVE_PROXIES.begin(), ProxyHandler::ALIVE_PROXIES.end());
    } else {
        return Stream(ProxyHandler::ALIVE_PROXIES.begin(), ProxyHandler::ALIVE_PROXIES.end()).filter([
            filter = std::move(filter)
        ](ProxyHandler* proxy) {
            switch (proxy->m_info->getURL().getOrigin()) {
                case Origin::GD: return filter == "Geometry Dash Server";
                case Origin::GD_CDN: return filter == "Geometry Dash CDN";
                case Origin::ROBTOP_GAMES: return filter == "RobtopGames Server";
                case Origin::NEWGROUNDS: return filter == "Newgrounds CDN";
                case Origin::GEODE: return filter == "Geode Server";
                case Origin::LOCALHOST: return filter == "Localhost";
                case Origin::LAN: return filter == "LAN";
                case Origin::OTHER: return filter == "Unknown Origin";
            }
        });
    }
}

bool ProxyHandler::isProxy(CCHttpRequest* request) {
    std::shared_lock aliveLock(ProxyHandler::ALIVE_MUTEX);

    return ProxyHandler::ALIVE_PROXIES.some([request](ProxyHandler* proxy) {
        return std::visit(makeVisitor{
            [request](CCHttpRequest* originalRequest) {
                return originalRequest == request;
            },
            [](const web::WebRequest& originalRequest) {
                return false;
            }
        }, proxy->m_info->m_request.m_original);
    });
}

bool ProxyHandler::isProxy(const web::WebRequest& request) {
    std::shared_lock aliveLock(ProxyHandler::ALIVE_MUTEX);

    return ProxyHandler::ALIVE_PROXIES.some([request](ProxyHandler* proxy) {
        return std::visit(makeVisitor{
            [](CCHttpRequest* originalRequest) {
                return false;
            },
            [request](const web::WebRequest& originalRequest) {
                return originalRequest.getID() == originalRequest.getID();
            }
        }, proxy->m_info->m_request.m_original);
    });
}

bool ProxyHandler::isPaused() {
    return Mod::get()->getSavedValue<bool>("paused");
}

void ProxyHandler::pauseAll() {
    Mod::get()->setSavedValue("paused", true);
}

// Thread unsafe
void ProxyHandler::resumeAll() {
    Mod::get()->setSavedValue("paused", false);
    std::vector<ProxyHandler*> paused;

    {
        std::lock_guard pauseLock(ProxyHandler::PAUSED_MUTEX);

        paused.swap(ProxyHandler::PAUSED_PROXIES);
    }

    for (ProxyHandler* proxy : paused) {
        // If a none cocos request or completed request gets here... wtf?
        proxy->m_info->m_request.resetTime();
        proxy->m_info->resume();
        CCHttpClient::getInstance()->send(std::get<CCHttpRequest*>(proxy->m_info->m_request.m_original));
    }
}

void ProxyHandler::setCacheLimit(const size_t limit) {
    Stream<ProxyHandler*> removedHandlers;

    {
        std::unique_lock aliveLock(ProxyHandler::ALIVE_MUTEX);

        if (ProxyHandler::ALIVE_PROXIES.size() > limit) {
            removedHandlers = ProxyHandler::ALIVE_PROXIES.slice(limit);

            ProxyHandler::ALIVE_PROXIES.resize(limit);
        }
    }

    removedHandlers.forEach([](ProxyHandler* handler) { delete handler; });
}

void ProxyHandler::registerProxy(ProxyHandler* proxy) {
    {
        std::unique_lock aliveLock(ProxyHandler::ALIVE_MUTEX);
        ProxyHandler::ALIVE_PROXIES = Stream({ proxy }).concat(ProxyHandler::ALIVE_PROXIES);

        if (ProxyHandler::ALIVE_PROXIES.size() > Mod::get()->getSettingValue<int64_t>("cache-limit")) {
            ProxyHandler* back = ProxyHandler::ALIVE_PROXIES.back();

            if (back->m_finished) {
                delete back;
            }

            ProxyHandler::ALIVE_PROXIES.pop_back();
        }
    }

    if (proxy->m_info->isPaused()) {
        std::lock_guard pauseLock(ProxyHandler::PAUSED_MUTEX);

        ProxyHandler::PAUSED_PROXIES.emplace_back(proxy);
    }

    ProxyEvent(EventState::REQUEST).send(proxy->m_info);
}

ProxyHandler::ProxyHandler(CCHttpRequest* request) : m_info(std::make_shared<HttpInfo>(request)),
m_originalTarget(request->getTarget()),
m_originalListener(request->getSelector()),
m_finished(false) {
    request->setResponseCallback(this, httpresponse_selector(ProxyHandler::onCocosResponse));

    if (!m_info->isPaused()) {
        m_info->m_request.resetTime();

        CCHttpClient::getInstance()->send(request);
    }
}

ProxyHandler::ProxyHandler(web::WebRequest& request) : m_info(std::make_shared<HttpInfo>(request)),
m_responseListener(web::IDBasedWebResponseEvent(request.getID()).listen([this](const web::WebResponse& response) {
    return this->onModResponse(response);
}, Priority::First)),
m_originalTarget(nullptr),
m_originalListener(nullptr),
m_finished(false) { }

std::shared_ptr<HttpInfo> ProxyHandler::getInfo() noexcept {
    return m_info;
}

void ProxyHandler::onCocosResponse(CCHttpClient* client, CCHttpResponse* response) {
    m_info->m_response.emplace(response, m_info.get());
    (m_originalTarget->*m_originalListener)(client, response);
    this->onResponse();
}

bool ProxyHandler::onModResponse(const web::WebResponse& response) {
    m_info->m_response.emplace(response, m_info.get());
    this->onResponse();

    return ListenerResult::Propagate;
}

void ProxyHandler::onResponse() {
    const int statusCode = m_info->m_response->getStatusCode();
    m_finished = true;

    if (statusCode == static_cast<int>(web::GeodeWebError::REQUEST_CANCELLED)) {
        m_info->m_state = State::CANCELLED;
    } else if (statusCode < 0) {
        m_info->m_state = State::FAULTY;
    } else {
        m_info->m_state = State::COMPLETED;
    }

    ProxyEvent(EventState::RESPONSE).send(m_info);

    {
        std::shared_lock aliveLock(ProxyHandler::ALIVE_MUTEX);

        if (std::find_if(ProxyHandler::ALIVE_PROXIES.begin(), ProxyHandler::ALIVE_PROXIES.end(), [this](ProxyHandler* handler) {
            return handler->m_info->getID() == m_info->getID();
        }) == ProxyHandler::ALIVE_PROXIES.end()) {
            delete this;
        }
    }
}