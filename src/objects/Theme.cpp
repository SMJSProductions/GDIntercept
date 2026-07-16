#include "Theme.hpp"

LookupTable<std::string, Theme::Theme> Theme::Theme::CACHED_THEMES;

CCLabelBMFont* Theme::Font::createLabel(const std::string_view text) const {
    CCLabelBMFont* label = CCLabelBMFont::create(StringBuffer(text).c_str(), fontName.c_str());

    label->setScale(fontScale);

    return label;
}

CCSize Theme::Font::getTrueFontSize() const {
    return this->createLabel("0")->getScaledContentSize();
}

void Theme::Theme::load() {
    Mod* mod = Mod::get();
    const std::filesystem::path themesDir = mod->getConfigDir() / "themes";

    (void) utils::file::createDirectoryAll(themesDir);

    Theme::CACHED_THEMES.clear();
    Theme::loadDirectory(mod->getResourcesDir());
    Theme::loadDirectory(themesDir);
    mod->setSavedValue("themes", Theme::CACHED_THEMES.keys());
}

void Theme::Theme::loadDirectory(const std::filesystem::path& path) {
    const Result<std::vector<std::filesystem::path>> dir = utils::file::readDirectory(path);

    ESCAPE_WHEN(dir.isErr(), log::warn("Failed to read directory {}: {}", path, dir.err().value_or("Unknown error")));

    for (const auto& file : dir.unwrap()) {
        CONTINUE_WHEN(file.extension() != ".json");

        const Result<std::string> text = utils::file::readString(file);

        if (text.isErr()) {
            log::warn("Failed to read theme file {}: {}", file, text.err().value_or("Unknown error"));
            continue;
        }

        const json theme = ordered_json::parse(text.unwrap());

        if (theme.contains("name") && theme.contains("theme")) try {
            Theme::CACHED_THEMES.insert(theme.at("name").get<std::string>(), Theme::createTheme(theme.at("theme")));
        } catch (const json::exception& e) {
            log::warn("Failed to parse theme file {}: {}", file, e.what());
        } else {
            log::info("Skipped parsing JSON file {}", file);
        }
    }
}

Theme::Theme Theme::Theme::createTheme(const json& obj) {
    const json code = obj.at("code");
    const json menu = obj.at("menu");
    const json codeFont = code.at("font");
    const json scrollbar = code.at("scrollbar");
    const json syntax = code.at("syntax");
    const json menuFont = menu.at("font");
    const Color thumb = Color::parse(scrollbar.at("thumb")).unwrapOrDefault();
    const Color syntaxText = Color::parse(syntax.at("text")).unwrapOrDefault();
    const Color syntaxKey = Color::parse(syntax.at("key")).unwrapOrDefault();
    const Color syntaxString = Color::parse(syntax.at("string")).unwrapOrDefault();
    const Color menuBackground = Color::parse(menu.at("background")).unwrapOrDefault();
    const Color menuText = Color::parse(menu.at("text")).unwrapOrDefault();
    const Color menuBorder = Color::parse(menu.at("border")).unwrapOrDefault();
    const Color disabledText = Theme::colorOrDefault(menu, "disabled-text", menuText, 0x55);
    const Color categoryText = Theme::colorOrDefault(menu, "category-text", menuText, 0xAA);
    const Menu::Icons defaultIcons = Menu::Icons({
        .info = Color::parse("#FF6961").unwrapOrDefault(),
        .body = Color::parse("#FFEE8C").unwrapOrDefault(),
        .query = Color::parse("#A2BFFE").unwrapOrDefault(),
        .header = Color::parse("#B19CD9").unwrapOrDefault(),
        .response = Color::parse("#B2FBA5").unwrapOrDefault(),
        .actionIcon = menuText,
        .disabledIcon = disabledText,
        .categoryIcon = categoryText
    });

    return {
        .code = {
            .paddingLeft = Theme::objectOrDefault<float>(code, "padding-left", [](const json& obj) { return obj.get<float>(); }, 3),
            .paddingRight = Theme::objectOrDefault<float>(code, "padding-right", [](const json& obj) { return obj.get<float>(); }, 3),
            .paddingCenter = Theme::objectOrDefault<float>(code, "padding-center", [](const json& obj) { return obj.get<float>(); }, 3),
            .background = Color::parse(code.at("background")).unwrapOrDefault(),
            .foreground = Color::parse(code.at("foreground")).unwrapOrDefault(),
            .font = {
                .fontName = codeFont.at("family").get<std::string>(),
                .fontScale = codeFont.at("scale").get<float>(),
                .lineHeight = Theme::objectOrDefault<float>(codeFont, "line-height", [](const json& obj) { return obj.get<float>(); }, 2)
            },
            .scrollbar = {
                .size = Theme::objectOrDefault<float>(scrollbar, "size", [](const json& obj) { return obj.get<float>(); }, 4),
                .border = Color::parse(scrollbar.at("border")).unwrapOrDefault(),
                .thumb = thumb,
                .activeThumb = Theme::colorOrDefault(scrollbar, "active-thumb", thumb, std::min<GLubyte>(thumb.a * 2, 255))
            },
            .syntax = {
                .text = syntaxText,
                .key = syntaxKey,
                .string = syntaxString,
                .number = Color::parse(syntax.at("number")).unwrapOrDefault(),
                .constant = Color::parse(syntax.at("constant")).unwrapOrDefault(),
                .error = Color::parse(syntax.at("error")).unwrapOrDefault(),
                .separator = Theme::colorOrDefault(syntax, "separator", syntaxText),
                .terminator = Theme::colorOrDefault(syntax, "terminator", syntaxText),
                .bracket = Theme::colorOrDefault(syntax, "bracket", syntaxText),
                .keyQuote = Theme::colorOrDefault(syntax, "key-quote", syntaxKey),
                .stringQuote = Theme::colorOrDefault(syntax, "string-quote", syntaxString)
            }
        },
        .menu = {
            .paddingLeft = Theme::objectOrDefault<float>(menu, "padding-left", [](const json& obj) { return obj.get<float>(); }, 3),
            .paddingRight = Theme::objectOrDefault<float>(menu, "padding-right", [](const json& obj) { return obj.get<float>(); }, 3),
            .paddingCenter = Theme::objectOrDefault<float>(menu, "padding-center", [](const json& obj) { return obj.get<float>(); }, 3),
            .background = Color::parse(menu.at("background")).unwrapOrDefault(),
            .categoryBackground = Theme::colorOrDefault(menu, "category-background", menuBackground),
            .foreground = Color::parse(menu.at("foreground")).unwrapOrDefault(),
            .text = menuText,
            .disabledText = disabledText,
            .categoryText = categoryText,
            .border = menuBorder,
            .categoryBorder = Theme::colorOrDefault(menu, "category-border", menuBorder),
            .font = {
                .fontName = menuFont.at("family").get<std::string>(),
                .fontScale = menuFont.at("scale").get<float>(),
                .lineHeight = Theme::objectOrDefault<float>(menuFont, "line-height", [](const json& obj) { return obj.get<float>(); }, 2)
            },
            .icons = Theme::objectOrDefault<Menu::Icons>(menu, "icons", [defaultIcons](const json& obj) {
                return Menu::Icons({
                    .info = Theme::colorOrDefault(obj, "info", defaultIcons.info),
                    .body = Theme::colorOrDefault(obj, "body", defaultIcons.body),
                    .query = Theme::colorOrDefault(obj, "query", defaultIcons.query),
                    .header = Theme::colorOrDefault(obj, "header", defaultIcons.header),
                    .response = Theme::colorOrDefault(obj, "response", defaultIcons.response),
                    .actionIcon = Theme::colorOrDefault(obj, "action-icon", defaultIcons.actionIcon),
                    .disabledIcon = Theme::colorOrDefault(obj, "disabled-icon", defaultIcons.disabledIcon),
                    .categoryIcon = Theme::colorOrDefault(obj, "category-icon", defaultIcons.categoryIcon)
                });
            }, defaultIcons)
        }
    };
}

Color Theme::Theme::colorOrDefault(const json& obj, const std::string_view key, const Color &override, const std::optional<GLubyte>& alpha) {
    if (obj.contains(key)) {
        if (Result<Color> color = Color::parse(obj.at(key)); color.ok()) return color.unwrap();
    }

    Color colorCopy = Color(override);

    if (alpha.has_value()) {
        colorCopy.a = alpha.value();
    }

    return colorCopy;
}

const Theme::Theme& Theme::getTheme() {
    const std::string theme = Mod::get()->getSettingValue<DynamicThemeValue>("theme");

    if (Theme::CACHED_THEMES.contains(theme)) {
        return Theme::CACHED_THEMES.at(std::move(theme));
    } else {
        // Dark is a default theme and will always exist unless someone messes with the default themes, in which case I can't guarantee anything
        return Theme::CACHED_THEMES.at("Dark");
    }
}