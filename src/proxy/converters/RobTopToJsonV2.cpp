#include "RobTopToJsonV2.hpp"

using namespace geode::prelude;
using namespace nlohmann;

const std::string proxy::converters::RobTopToJsonV2::Object::METADATA_DELIMITER = "#";

// NOTE: Some responses are a simple object without metadata. For both consistency and reliability sake
// add a metadata delimiter parser on top of it to prevent trailing data if the response gets metadata some day.
const std::unordered_map<proxy::converters::RobTopToJsonV2::ObjectType, proxy::converters::RobTopToJsonV2::Object> proxy::converters::RobTopToJsonV2::PARSERS({
    { ObjectType::AS_IS, Object() },
    { ObjectType::AS_IS_B64, Object(Encodings::B64) },
    { ObjectType::OBJECT, Object(",") },
    { ObjectType::ARRAY, Object(",", ObjectType::AS_IS) },
    { ObjectType::COLOR, Object(",", Object::Tuple{
        { "r", ObjectType::AS_IS },
        { "g", ObjectType::AS_IS },
        { "b", ObjectType::AS_IS }
    }) },
    { ObjectType::PAGINATION, Object(":",Object::Tuple{
        { "total", ObjectType::AS_IS },
        { "offset", ObjectType::AS_IS },
        { "amount", ObjectType::AS_IS }
    }) },
    { ObjectType::SYNC_ACCOUNT_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "save", Object({ Encodings::B64_GZIP_XOR, false }) }
    }) },
    { ObjectType::LOGIN_ACCOUNT_RESPONSE, Object(",", Object::Tuple{
        { "accountID", ObjectType::AS_IS },
        { "userID", ObjectType::AS_IS }
    }) },
    { ObjectType::USER_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "users", Object("|", ObjectType::USER) },
        { "pagination", ObjectType::PAGINATION }
    }) },
    { ObjectType::USER_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "user", ObjectType::USER }
    }) },
    { ObjectType::USER, Object(":", Object::Mappings{
        { "55", Object(",", Object::Tuple{
            { "easyDemonCompletions", ObjectType::AS_IS },
            { "mediumDemonCompletions", ObjectType::AS_IS },
            { "hardDemonCompletions", ObjectType::AS_IS },
            { "insaneDemonCompletions", ObjectType::AS_IS },
            { "extremeDemonCompletions", ObjectType::AS_IS },
            { "easyPlatformerDemonCompletions", ObjectType::AS_IS },
            { "mediumPlatformerDemonCompletions", ObjectType::AS_IS },
            { "hardPlatformerDemonCompletions", ObjectType::AS_IS },
            { "insanePlatformerDemonCompletions", ObjectType::AS_IS },
            { "extremePlatformerDemonCompletions", ObjectType::AS_IS },
            { "weeklyDemonCompletions", ObjectType::AS_IS },
            { "gauntletDemonCompletions", ObjectType::AS_IS }
        }) },
        { "56", Object(",", Object::Tuple{
            { "autoCompletions", ObjectType::AS_IS },
            { "easyCompletions", ObjectType::AS_IS },
            { "normalCompletions", ObjectType::AS_IS },
            { "hardCompletions", ObjectType::AS_IS },
            { "harderCompletions", ObjectType::AS_IS },
            { "insaneCompletions", ObjectType::AS_IS },
            { "dailyCompletions", ObjectType::AS_IS },
            { "gauntletCompletions", ObjectType::AS_IS }
        }) },
        { "57", Object(",", Object::Tuple{
            { "autoPlatformerCompletions", ObjectType::AS_IS },
            { "easyPlatformerCompletions", ObjectType::AS_IS },
            { "normalPlatformerCompletions", ObjectType::AS_IS },
            { "hardPlatformerCompletions", ObjectType::AS_IS },
            { "harderPlatformerCompletions", ObjectType::AS_IS },
            { "insanePlatformerCompletions", ObjectType::AS_IS },
            { "insanePlatformerCompletions", ObjectType::AS_IS },
            { "insanePlatformerCompletions", ObjectType::AS_IS },
            { "theMapCompletions", ObjectType::AS_IS }
        }) }
    }) },
    { ObjectType::MESSAGE_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "messages", Object("|", ObjectType::MESSAGE) },
        { "pagination", ObjectType::PAGINATION }
    }) },
    { ObjectType::MESSAGE_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "message", ObjectType::MESSAGE }
    }) },
    { ObjectType::MESSAGE, Object(":", Object::Mappings{
        { "4", ObjectType::AS_IS_B64 },
        { "5", Object({ Encodings::B64_XOR, 14251 }) }
    }) },
    { ObjectType::COMMENT_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "comments", Object("|", ObjectType::COMMENT) },
        { "pagination", ObjectType::PAGINATION }
    }) },
    { ObjectType::COMMENT, Object(":", Object::Tuple{
        { "content", Object("~", Object::Mappings{
                { "2", ObjectType::AS_IS_B64 }
            }) },
        { "sender", Object("~") }
    }) },
    { ObjectType::MAP_PACK_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "packs", ObjectType::LEVEL_PACK_ARRAY },
        { "pagination", ObjectType::PAGINATION },
        { "hash", ObjectType::AS_IS }
    }) },
    { ObjectType::GAUNTLET_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "gauntlets", ObjectType::LEVEL_PACK_ARRAY },
        { "hash", ObjectType::AS_IS }
    }) },
    { ObjectType::LIST_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "lists", ObjectType::LEVEL_ARRAY },
        { "creators", ObjectType::CREATOR_ARRAY },
        { "pagination", ObjectType::PAGINATION },
        { "hash", ObjectType::AS_IS }
    }) },
    { ObjectType::LEVEL_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "levels", ObjectType::LEVEL_ARRAY },
        { "creators", ObjectType::CREATOR_ARRAY },
        { "songs", ObjectType::SONG_ARRAY },
        { "pagination", ObjectType::PAGINATION },
        { "hash", ObjectType::AS_IS }
    }) },
    { ObjectType::LEVEL_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "level", ObjectType::LEVEL },
        { "hash1", ObjectType::AS_IS },
        { "hash2", ObjectType::AS_IS },
        { "dailyCreator", ObjectType::CREATOR },
        { "songs", ObjectType::SONG_ARRAY }
    }) },
    { ObjectType::DAILY_RESPONSE, Object("|", Object::Tuple{
        { "tempID", ObjectType::AS_IS },
        { "secondsLeft", ObjectType::AS_IS }
    }) },
    { ObjectType::LEVEL_PACK_ARRAY, Object("|", ObjectType::LEVEL_PACK) },
    { ObjectType::LEVEL_PACK, Object(":", Object::Mappings{
        { "3", ObjectType::ARRAY },
        { "7", ObjectType::COLOR },
        { "8", ObjectType::COLOR }
    }) },
    { ObjectType::LEVEL_ARRAY, Object("|", ObjectType::LEVEL) },
    { ObjectType::LEVEL, Object(":", Object::Mappings{
        { "3", ObjectType::AS_IS_B64 },
        { "4", Object({ Encodings::B64_GZIP_XOR, false }) },
        { "27", Object({ Encodings::B64_XOR, 26364 }) },
        { "36", ObjectType::OBJECT },
        { "51", ObjectType::ARRAY },
        { "52", ObjectType::ARRAY },
        { "53", ObjectType::ARRAY }
    }) },
    { ObjectType::CREATOR_ARRAY, Object("|", ObjectType::CREATOR) },
    { ObjectType::CREATOR, Object(":", Object::Tuple{
        { "userID", ObjectType::AS_IS },
        { "username", ObjectType::AS_IS },
        { "accountID", ObjectType::AS_IS }
    }) },
    { ObjectType::LEVEL_SCORE_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "scores", Object("|", ObjectType::LEVEL_SCORE) }
    }) },
    { ObjectType::LEVEL_SCORE, Object(":") },
    { ObjectType::TOP_ARTIST_ARRAY_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "artists", Object("|", ObjectType::TOP_ARTIST) },
        { "pagination", ObjectType::PAGINATION }
    }) },
    { ObjectType::TOP_ARTIST, Object(":") },
    { ObjectType::SONG_INFO_RESPONSE, Object(Object::METADATA_DELIMITER, Object::Tuple{
        { "song", ObjectType::SONG }
    }) },
    { ObjectType::SONG_ARRAY, Object("~:~", ObjectType::SONG) },
    { ObjectType::SONG, Object("~|~") },
    { ObjectType::CHALLENGE_RESPONSE, Object("|", Object::Tuple{
        { "challenges", Object(":", Object::Tuple{
            { "randomHash", ObjectType::AS_IS },
            { "uuid", ObjectType::AS_IS },
            { "decodedChk", ObjectType::AS_IS },
            { "udid", ObjectType::AS_IS },
            { "accountID", ObjectType::AS_IS },
            { "secondsLeft", ObjectType::AS_IS },
            { "queuedQuests", ObjectType::CHALLENGE, true }
        }, { Encodings::B64_XOR, 19847, 5 }) },
        { "hash", ObjectType::AS_IS }
    }) },
    { ObjectType::CHALLENGE, Object(",", Object::Tuple{
        { "id", ObjectType::AS_IS },
        { "type", ObjectType::AS_IS },
        { "goal", ObjectType::AS_IS },
        { "reward", ObjectType::AS_IS },
        { "name", ObjectType::AS_IS }
    }) },
    { ObjectType::REWARD_RESPONSE, Object("|", Object::Tuple{
        { "rewards", Object(":", Object::Tuple{
            { "randomHash", ObjectType::AS_IS },
            { "uuid", ObjectType::AS_IS },
            { "decodedChk", ObjectType::AS_IS },
            { "udid", ObjectType::AS_IS },
            { "accountID", ObjectType::AS_IS },
            { "smallChestSecondsLeft", ObjectType::AS_IS },
            { "smallChestRewards", ObjectType::REWARD },
            { "smallChestsClaimedBefore", ObjectType::AS_IS },
            { "largeChestSecondsLeft", ObjectType::AS_IS },
            { "largeChestRewards", ObjectType::REWARD },
            { "largeChestsClaimedBefore", ObjectType::AS_IS },
            { "requestType", ObjectType::AS_IS }
        }, { Encodings::B64_XOR, 59182, 5 }) },
        { "hash", ObjectType::AS_IS },
    }) },
    { ObjectType::REWARD, Object(",", Object::Tuple{
        { "orbs", ObjectType::AS_IS },
        { "diamonds", ObjectType::AS_IS },
        { "shardTypes", ObjectType::AS_IS, true },
    }) },
    { ObjectType::WRAITH_RESPONSE, Object("|", Object::Tuple{
        { "reward", Object(":", Object::Tuple{
            { "randomHash", ObjectType::AS_IS },
            { "decodedChk", ObjectType::AS_IS },
            { "rewardID", ObjectType::AS_IS },
            { "chestType", ObjectType::AS_IS },
            { "rewards", Object(",") }
        }, { Encodings::B64_XOR, 59182, 5 }) },
        { "hash", ObjectType::AS_IS }
    }) }
});

const std::unordered_map<std::string, proxy::converters::RobTopToJsonV2::ObjectType> proxy::converters::RobTopToJsonV2::ENDPOINT_MAPPINGS({
    { "syncGJAccountNew.php", ObjectType::SYNC_ACCOUNT_RESPONSE },
    { "getSaveData.php", ObjectType::SYNC_ACCOUNT_RESPONSE },
    { "loginGJAccount.php", ObjectType::LOGIN_ACCOUNT_RESPONSE },
    { "getGJFriendRequests20.php", ObjectType::USER_ARRAY_RESPONSE },
    { "getGJUserList20.php", ObjectType::USER_ARRAY_RESPONSE },
    { "getGJUsers20.php", ObjectType::USER_ARRAY_RESPONSE },
    { "getGJUserInfo20.php", ObjectType::USER_RESPONSE },
    { "getGJMessages20.php", ObjectType::MESSAGE_ARRAY_RESPONSE },
    { "downloadGJMessage20.php", ObjectType::MESSAGE_RESPONSE },
    { "getGJAccountComments20.php", ObjectType::COMMENT_ARRAY_RESPONSE },
    { "getGJCommentHistory.php", ObjectType::COMMENT_ARRAY_RESPONSE },
    { "getGJComments21.php", ObjectType::COMMENT_ARRAY_RESPONSE },
    { "getGJMapPacks21.php", ObjectType::MAP_PACK_ARRAY_RESPONSE },
    { "getGJGauntlets21.php", ObjectType::GAUNTLET_ARRAY_RESPONSE },
    { "getGJLevelLists.php", ObjectType::LIST_ARRAY_RESPONSE },
    { "getGJLevels21.php", ObjectType::LEVEL_ARRAY_RESPONSE },
    { "downloadGJLevel22.php", ObjectType::LEVEL_RESPONSE },
    { "getGJDailyLevel.php", ObjectType::DAILY_RESPONSE },
    { "getGJScores20.php", ObjectType::USER_ARRAY_RESPONSE },
    { "getGJLevelScores211.php", ObjectType::LEVEL_SCORE_ARRAY_RESPONSE },
    { "getGJLevelScoresPlat.php", ObjectType::LEVEL_SCORE_ARRAY_RESPONSE },
    { "getGJTopArtists.php", ObjectType::TOP_ARTIST_ARRAY_RESPONSE },
    { "getGJSongInfo.php", ObjectType::SONG_INFO_RESPONSE },
    { "getGJChallenges.php", ObjectType::CHALLENGE_RESPONSE },
    { "getGJRewards.php", ObjectType::REWARD_RESPONSE },
    { "getGJSecretReward.php", ObjectType::WRAITH_RESPONSE }
});

proxy::converters::RobTopToJsonV2::DecodeStrategy::DecodeStrategy(const Encodings encodings, const int key, const size_t skip) :
encodings(encodings),
key(key),
skip(skip) { }

proxy::converters::RobTopToJsonV2::Object::Object(const DecodeStrategy decodeStrategy) :
m_strategy(Strategy::PRIMITIVE),
m_delimiter(),
m_entryType(ObjectType::AS_IS),
m_decodeStrategy(decodeStrategy) { }

proxy::converters::RobTopToJsonV2::Object::Object(std::string delimiter, Mappings mappings, const DecodeStrategy decodeStrategy) :
m_strategy(Strategy::OBJECT_SPLIT),
m_delimiter(std::move(delimiter)),
m_entryType(ObjectType::AS_IS),
m_decodeStrategy(decodeStrategy),
m_mappings(std::move(mappings)) { }

proxy::converters::RobTopToJsonV2::Object::Object(std::string entryDelimiter, const ObjectType entryType, const DecodeStrategy decodeStrategy) :
m_strategy(Strategy::ARRAY_SPLIT),
m_delimiter(std::move(entryDelimiter)),
m_entryType(entryType),
m_decodeStrategy(decodeStrategy) { }

proxy::converters::RobTopToJsonV2::Object::Object(std::string entryDelimiter, Tuple tuple, const DecodeStrategy decodeStrategy) :
m_strategy(Strategy::TUPLE_SPLIT),
m_delimiter(std::move(entryDelimiter)),
m_entryType(ObjectType::AS_IS),
m_tuple(std::move(tuple)),
m_decodeStrategy(decodeStrategy) { }

ordered_json proxy::converters::RobTopToJsonV2::Object::parse(std::string_view original, const bool censor) const {
    // This exists here to give the decoded result a lifetime
    std::string decodedOriginal;

    // Escape if it shouldn't decode or is a number like -1 which may result in incorrect parsing for failed responses
    if ((m_decodeStrategy.encodings != Encodings::NONE && !Mod::get()->getSettingValue<bool>("decode-data")) || converters::isNumber(original)) {
        return converters::getPrimitiveJsonType({}, original, censor);
    }

    switch (m_decodeStrategy.encodings) {
        case Encodings::B64: {
            auto result = utils::base64::decodeString(original);

            if (result.isOk()) {
                original = decodedOriginal = std::move(result).unwrap();
            }
        } break;
        case Encodings::B64_XOR: {
            StringBuffer decodeString(m_decodeStrategy.skip ? original.substr(m_decodeStrategy.skip) : original);

            original = decodedOriginal = cocos2d::ZipUtils::base64DecodeEnc(gd::string(decodeString.c_str(), decodeString.size()), std::to_string(m_decodeStrategy.key));
        } break;
        case Encodings::B64_GZIP_XOR: {
            StringBuffer decodeString(m_decodeStrategy.skip ? original.substr(m_decodeStrategy.skip) : original);

            original = decodedOriginal = cocos2d::ZipUtils::decompressString(
                gd::string(decodeString.c_str(), decodeString.size()),
                m_decodeStrategy.key,
                m_decodeStrategy.key
            );
        } break;
        // Clang will otherwise warn about fall-through
        case Encodings::NONE: break;
    }

    switch(m_strategy) {
        case Strategy::PRIMITIVE: return converters::getPrimitiveJsonType({}, original, censor);
        case Strategy::OBJECT_SPLIT: return this->parseObject(original, censor);
        case Strategy::ARRAY_SPLIT: return this->parseArray(original, censor);
        case Strategy::TUPLE_SPLIT: return this->parseTuple(original, censor);
    };
}

ordered_json proxy::converters::RobTopToJsonV2::Object::parseObject(const std::string_view original, const bool censor) const {
    ordered_json object = ordered_json::object();
    std::string activeKey;

    if (original.empty()) {
        return object;
    }

    StringUtils::split(original, m_delimiter).forEach([this, &activeKey, &object, censor](std::string&& part, const size_t i) {
        if (i % 2 == 0) {
            activeKey = std::move(part);
        } else if (const Mappings::const_iterator it = m_mappings.find(activeKey); it != m_mappings.end()) {
            const Object& mapping = this->getObject(it->second);

            object[std::move(activeKey)] = mapping.parse(std::move(part), censor);
        } else {
            nlohmann::json value = converters::getPrimitiveJsonType(activeKey, std::move(part), censor);

            object[std::move(activeKey)] = std::move(value);
        }
    });

    return object;
}

ordered_json proxy::converters::RobTopToJsonV2::Object::parseArray(const std::string_view original, const bool censor) const {
    ordered_json array = ordered_json::array();

    if (original.empty()) {
        return array;
    }

    StringUtils::split(original, m_delimiter).forEach([this, &array, censor](std::string&& part) {
        array.emplace_back(RobTopToJsonV2::PARSERS.at(m_entryType).parse(std::move(part), censor));
    });

    return array;
}

ordered_json proxy::converters::RobTopToJsonV2::Object::parseTuple(const std::string_view original, const bool censor) const {
    const size_t tupleSize = m_tuple.size();
    ordered_json object = ordered_json::object();
    size_t unknownCount = 0;

    if (original.empty()) {
        return object;
    }

    StringUtils::split(original, m_delimiter).forEach([this, tupleSize, &object, &unknownCount, censor](std::string&& part, const size_t i) {
        const bool withinSize = i < tupleSize;

        if (withinSize || (tupleSize && m_tuple.back().vararg)) {
            const auto& fieldInfo = withinSize ? m_tuple.at(i) : m_tuple.back();
            ordered_json value = this->getObject(fieldInfo.source).parse(std::move(part), censor);
            ordered_json& field = object[fieldInfo.key];

            if (!fieldInfo.vararg) {
                field = std::move(value);
            } else if (object.contains(fieldInfo.key)) {
                field.emplace_back(std::move(value));
            } else {
                field = ordered_json::array({ std::move(value) });
            }
        } else {
            object[fmt::format("unknown-{}", ++unknownCount)] = converters::getPrimitiveJsonType({}, std::move(part), censor);
        }
    });

    return object;
}

proxy::converters::RobTopToJsonV2::RobTopToJsonV2() : Converter(enums::ContentType::JSON) { };

bool proxy::converters::RobTopToJsonV2::canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const {
    return !isRequest && RobTopToJsonV2::ENDPOINT_MAPPINGS.contains(this->splitPath(path));
}

std::string proxy::converters::RobTopToJsonV2::convert(const std::string_view path, const std::string_view original, const bool censor) const {
    return converters::safeDump(RobTopToJsonV2::PARSERS.at(RobTopToJsonV2::ENDPOINT_MAPPINGS.at(this->splitPath(path))).parse(original, censor));
}

std::string proxy::converters::RobTopToJsonV2::toRaw(const std::string_view path, const std::string_view original) const {
    // TODO: Actually implement this
    return std::string(original);
}