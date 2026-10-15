#pragma once

#include <Geode/utils/base64.hpp>
#include "Converter.hpp"

namespace proxy::converters {
    class RobTopToJsonV2 : public Converter {
        enum class ObjectType {
            AS_IS,
            AS_IS_B64,
            OBJECT,
            ARRAY,
            COLOR,
            PAGINATION,
            SYNC_ACCOUNT_RESPONSE,
            LOGIN_ACCOUNT_RESPONSE,
            USER_ARRAY_RESPONSE,
            USER_RESPONSE,
            USER,
            MESSAGE_ARRAY_RESPONSE,
            MESSAGE_RESPONSE,
            MESSAGE,
            COMMENT_ARRAY_RESPONSE,
            COMMENT,
            MAP_PACK_ARRAY_RESPONSE,
            GAUNTLET_ARRAY_RESPONSE,
            LIST_ARRAY_RESPONSE,
            LEVEL_ARRAY_RESPONSE,
            LEVEL_RESPONSE,
            DAILY_RESPONSE,
            LEVEL_PACK_ARRAY,
            LEVEL_PACK,
            LEVEL_ARRAY,
            LEVEL,
            CREATOR_ARRAY,
            CREATOR,
            LEVEL_SCORE_ARRAY_RESPONSE,
            LEVEL_SCORE,
            TOP_ARTIST_ARRAY_RESPONSE,
            TOP_ARTIST,
            SONG_INFO_RESPONSE,
            SONG_ARRAY,
            SONG,
            CHALLENGE_RESPONSE,
            CHALLENGE,
            REWARD_RESPONSE,
            REWARD,
            WRAITH_RESPONSE
        };

        enum class Encodings {
            NONE,
            B64,
            B64_XOR,
            B64_GZIP_XOR
        };

        enum class Strategy {
            PRIMITIVE,
            OBJECT_SPLIT,
            ARRAY_SPLIT,
            TUPLE_SPLIT
        };

        struct DecodeStrategy {
            const Encodings encodings;
            const int key;
            const size_t skip;

            DecodeStrategy(const Encodings encodings, const int key = 0, const size_t skip = 0);
        };

        struct TupleField;

        class Object {
        public:
            using ObjectSource = std::variant<ObjectType, Object>;
            using Mappings = std::unordered_map<std::string, ObjectSource>;
            using Tuple = std::vector<TupleField>;

            static const std::string METADATA_DELIMITER;

            Object(const DecodeStrategy decodeStrategy = Encodings::NONE);
            Object(std::string delimiter, Mappings mappings = {}, const DecodeStrategy decodeStrategy = Encodings::NONE);
            Object(std::string entryDelimiter, const ObjectType entryType, const DecodeStrategy decodeStrategy = Encodings::NONE);
            Object(std::string entryDelimiter, Tuple tuple, const DecodeStrategy decodeStrategy = Encodings::NONE);
            nlohmann::ordered_json parse(std::string_view original, const bool censor) const;
        private:
            const Strategy m_strategy;
            const std::string m_delimiter;
            const ObjectType m_entryType;
            const Tuple m_tuple;
            const DecodeStrategy m_decodeStrategy;
            const Mappings m_mappings;

            nlohmann::ordered_json parseObject(const std::string_view original, const bool censor) const;
            nlohmann::ordered_json parseArray(const std::string_view original, const bool censor) const;
            nlohmann::ordered_json parseTuple(const std::string_view original, const bool censor) const;

            inline const Object& getObject(const ObjectSource& source) const {
                if (std::holds_alternative<Object>(source)) {
                    return std::get<Object>(source);
                } else {
                    return RobTopToJsonV2::PARSERS.at(std::get<ObjectType>(source));
                }
            }
        };

        struct TupleField {
            const std::string key;
            const Object::ObjectSource source;
            // Ignored if not the last tuple field
            const bool vararg = false;
        };
    public:
        RobTopToJsonV2();
        bool canConvert(const std::string_view path, const bool isRequest, const std::string_view original) const override;
        std::string convert(const std::string_view path, const std::string_view original, const bool censor) const override;
        std::string toRaw(const std::string_view path, const std::string_view original) const override;
    private:
        static const std::unordered_map<ObjectType, Object> PARSERS;
        static const std::unordered_map<std::string, ObjectType> ENDPOINT_MAPPINGS;

        inline std::string splitPath(const std::string_view path) const {
            return StringUtils::split(path, "/").back();
        }
    };
}