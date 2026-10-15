#pragma once

namespace proxy::enums {
    enum class Client {
        COCOS,
        GEODE
    };

    enum class ContentType {
        FORM,
        JSON,
        XML,
        ROBTOP,
        BINARY,
        UNKNOWN_CONTENT
    };

    enum class Origin {
        GD,
        GD_CDN,
        ROBTOP_GAMES,
        NEWGROUNDS,
        GEODE,
        LOCALHOST,
        LAN,
        OTHER
    };

    enum class Scheme {
        HTTP,
        HTTPS,
        WS,
        WSS,
        FTP,
        SFTP,
        FILE,
        STEAM,
        MAIL,
        // Any other schemes are either custom or make 0 sense in the context of a game
        UNKNOWN_SCHEME
    };

    enum class State {
        PAUSED,
        IN_PROGRESS,
        COMPLETED,
        FAULTY,
        CANCELLED
    };

    enum class OriginFilter {
        GD_FILTER,
        GD_CDN_FILTER,
        ROBTOP_GAMES_FILTER,
        NEWGROUNDS_FILTER,
        GEODE_FILTER,
        LOCALHOST_FILTER,
        OTHER_FILTER,
        ALL_FILTER
    };

    enum class EventState {
        REQUEST,
        PROGRESS,
        RESPONSE,
        CANCEL
    };
}