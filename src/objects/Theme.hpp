#pragma once

#include <json.hpp>
#include <LookupTable.hpp>
#include <Geode/Geode.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/ColorCode.hpp>
#include "../macro_utils.hpp"
#include "../settings/DynamicTheme.hpp"

using namespace nlohmann;
using namespace geode::prelude;

template<typename T>
concept ColorNode = requires(T color) {
    { color.setOpacity(float()) };
    { color.setColor(ccColor3B()) };
};

namespace Theme {
    struct Font {
        std::string fontName;
        float fontScale;
        float lineHeight;

        CCLabelBMFont* createLabel(const ZStringView text) const;
        CCSize getFontSize() const;
        CCSize getTrueFontSize() const;
    };

    struct Code {
        struct Scrollbar {
            float size;
            Color border;
            Color thumb;
            Color activeThumb;
        };

        struct Syntax {
            Color text;
            Color key;
            Color string;
            Color number;
            Color constant;
            Color error;
            Color separator;
            Color terminator;
            Color bracket;
            Color keyQuote;
            Color stringQuote;
        };

        float paddingLeft;
        float paddingRight;
        float paddingCenter;
        Color background;
        Color foreground;
        Font font;
        Scrollbar scrollbar;
        Syntax syntax;
    };

    struct Menu {
        struct Icons {
            Color info;
            Color body;
            Color query;
            Color header;
            Color response;
            Color actionIcon;
            Color disabledIcon;
            Color categoryIcon;
        };

        float paddingLeft;
        float paddingRight;
        float paddingCenter;
        Color background;
        Color categoryBackground;
        Color foreground;
        Color text;
        Color disabledText;
        Color categoryText;
        Color border;
        Color categoryBorder;
        Font font;
        Icons icons;
    };

    struct Theme {
        static void load();

        Code code;
        Menu menu;
    private:
        static LookupTable<std::string, Theme> CACHED_THEMES;

        static void loadDirectory(const std::filesystem::path& path);
        static Theme createTheme(const json& obj);
        static Color colorOrDefault(const json& obj, const std::string_view key, const Color& override, const std::optional<GLubyte>& alpha = std::nullopt);

        template <typename T>
        static T objectOrDefault(const json& obj, const std::string_view key, const std::function<T(const json& obj)>& constructor, const T& defaultObj) {
            return obj.contains(key) ? constructor(obj.at(key)) : defaultObj;
        }

        friend const Theme& getTheme();
    };

    const Theme& getTheme();
};