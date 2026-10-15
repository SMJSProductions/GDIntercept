#include "DynamicTheme.hpp"

bool DynamicThemeValue::operator==(const DynamicThemeValue& other) const {
    return value == other.value;
}

DynamicThemeValue::operator std::string() const {
    return value;
}

DynamicThemeValue::DynamicThemeValue(std::string value) : value(std::move(value)) { }

DynamicThemeValue::DynamicThemeValue(const DynamicThemeValue& other) : value(other.value) { }

matjson::Value matjson::Serialize<DynamicThemeValue>::to_json(const DynamicThemeValue& value) {
    return value.value;
}

DynamicThemeValue matjson::Serialize<DynamicThemeValue>::from_json(const matjson::Value& json) {
    Result<std::string> result = json.asString();

    if (result) {
        return DynamicThemeValue(result.unwrap());
    } else {
        return DynamicThemeValue();
    }
}

bool matjson::Serialize<DynamicThemeValue>::is_json(const matjson::Value& json) {
    return json.isString();
}

StringMap<Stream<std::function<void()>>> DynamicTheme::LOADERS;

Result<std::shared_ptr<SettingV3>> DynamicTheme::parse(std::string key, std::string modID, const matjson::Value& json) {
    std::shared_ptr<DynamicTheme> res = std::make_shared<DynamicTheme>();
    JsonExpectedValue root = checkJson(json, "DynamicTheme");

    res->parseBaseProperties(std::move(key), std::move(modID), root);
    root.needs("save-value").into(res->m_saveValue);
    root.checkUnknownKeys();

    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

void DynamicTheme::reloadDynamicThemes(std::string sprSettingKey) {
    DynamicTheme::LOADERS.at(std::move(sprSettingKey)).forEach([](const auto& loader) { loader(); });
}

SettingNodeV3* DynamicTheme::createNode(const float width) {
    return DynamicThemeNode::create(std::static_pointer_cast<DynamicTheme>(shared_from_this()), width);
}

std::string DynamicTheme::getSaveValue() const {
    return m_saveValue;
}

DynamicThemeNode* DynamicThemeNode::create(const std::shared_ptr<DynamicTheme>& setting, const float width) {
    DynamicThemeNode* instance = new DynamicThemeNode();

    if (instance && instance->init(setting, width)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

bool DynamicThemeNode::init(const std::shared_ptr<DynamicTheme>& setting, const float width) {
    if (!SettingValueNodeV3::init(setting, width)) {
        return false;
    }

    const float height = this->getContentHeight();
    const float buttonYOffsets = height / -1.25f;
    const float newHeight = height + -buttonYOffsets;
    const float deltaHeight = (newHeight - height) / 2;
    CCMenu* nameMenu = this->getNameMenu();
    CCMenu* buttonMenu = this->getButtonMenu();
    m_label = CCLabelBMFont::create(this->getSetting()->getValue().value.c_str(), "bigFont.fnt");
    m_arrowLeftSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    m_arrowRightSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    CCSprite* refreshSpr = CCSprite::createWithSpriteFrameName("GJ_getSongInfoBtn_001.png");
    CCMenuItemSpriteExtra* refreshBtn = CCMenuItemSpriteExtra::create(refreshSpr, this, menu_selector(DynamicThemeNode::onRefresh));
    CCMenuItemSpriteExtra* arrowLeftBtn = CCMenuItemSpriteExtra::create(m_arrowLeftSpr, this, menu_selector(DynamicThemeNode::onArrow));
    CCMenuItemSpriteExtra* arrowRightBtn = CCMenuItemSpriteExtra::create(m_arrowRightSpr, this, menu_selector(DynamicThemeNode::onArrow));
    ButtonSprite* filesSpr = ButtonSprite::create("Open Files", "bigFont.fnt", "GJ_button_01.png");
    ButtonSprite* docsSpr = ButtonSprite::create("Open Docs", "bigFont.fnt", "GJ_button_01.png");
    CCMenuItemSpriteExtra* filesBtn = CCMenuItemSpriteExtra::create(filesSpr, this, menu_selector(DynamicThemeNode::onFiles));
    CCMenuItemSpriteExtra* docsBtn = CCMenuItemSpriteExtra::create(docsSpr, this, menu_selector(DynamicThemeNode::onDocs));

    m_arrowLeftSpr->setFlipX(true);
    m_arrowLeftSpr->setScaleX(0.4f);
    m_arrowLeftSpr->setScaleY(0.25f);
    m_arrowLeftSpr->setPosition(m_arrowLeftSpr->getScaledContentSize() / 2);
    m_arrowRightSpr->setScaleX(0.4f);
    m_arrowRightSpr->setScaleY(0.25f);
    m_arrowRightSpr->setPosition(m_arrowRightSpr->getScaledContentSize() / 2);
    refreshSpr->setScale(0.5f);
    refreshSpr->setPosition(refreshSpr->getScaledContentSize() / 2);
    filesSpr->setScale(0.4f);
    filesSpr->setPosition(filesSpr->getScaledContentSize() / 2);
    docsSpr->setScale(0.4f);
    docsSpr->setPosition(docsSpr->getScaledContentSize() / 2);
    arrowLeftBtn->setTag(-1);
    arrowLeftBtn->setContentSize(m_arrowLeftSpr->getScaledContentSize());
    arrowRightBtn->setTag(1);
    arrowRightBtn->setContentSize(m_arrowRightSpr->getScaledContentSize());
    refreshBtn->setContentSize(refreshSpr->getScaledContentSize());
    filesBtn->setContentSize(filesSpr->getScaledContentSize());
    docsBtn->setContentSize(docsSpr->getScaledContentSize());

    buttonMenu->addChildAtPosition(filesBtn, Anchor::Left, { width / -2 + 5, buttonYOffsets });
    buttonMenu->addChildAtPosition(docsBtn, Anchor::Right, { docsSpr->getScaledContentWidth() / -2, buttonYOffsets });
    buttonMenu->addChildAtPosition(m_label, Anchor::Center);
    buttonMenu->addChildAtPosition(arrowLeftBtn, Anchor::Left, { 5, 0 });
    buttonMenu->addChildAtPosition(arrowRightBtn, Anchor::Right, { -5, 0 });
    buttonMenu->addChildAtPosition(refreshBtn, Anchor::Left, { -20, 0 });

    nameMenu->updateAnchoredPosition(Anchor::Left, { 10, deltaHeight });
    buttonMenu->updateAnchoredPosition(Anchor::Right, { -10, deltaHeight });

    this->setContentHeight(50);
    this->updateState(nullptr);

    return true;
}

void DynamicThemeNode::updateState(CCNode* invoker) {
    SettingValueNodeV3::updateState(invoker);
    const bool enabled = this->getSetting()->shouldEnable();

    if (invoker != m_label) {
        m_label->setCString(this->getValue().value.c_str());
    }

    m_label->limitLabelWidth(88, 0.44f, 0.05f);
    m_arrowLeftSpr->setOpacity(enabled ? 255 : 100);
    m_arrowLeftSpr->setColor(enabled ? ccWHITE : ccGRAY);
    m_arrowRightSpr->setOpacity(enabled ? 255 : 100);
    m_arrowRightSpr->setColor(enabled ? ccWHITE : ccGRAY);
}

void DynamicThemeNode::onArrow(CCObject* sender) {
    const std::vector<std::string> options = Mod::get()->getSavedValue<std::vector<std::string>>(this->getSetting()->getSaveValue());
    size_t index = ranges::indexOf(options, this->getValue().value).value_or(0);

    if (sender->getTag() > 0) {
        index = index < options.size() - 1 ? index + 1 : 0;
    } else {
        index = index > 0 ? index - 1 : options.size() - 1;
    }

    this->setValue(options.at(index), typeinfo_cast<CCNode*>(sender));
}

void DynamicThemeNode::onRefresh(CCObject* sender) {
    const std::shared_ptr<DynamicTheme> setting = this->getSetting();

    m_label->setCString(this->getValue().value.c_str());

    DynamicTheme::reloadDynamicThemes(fmt::format("{}/{}", setting->getModID(), setting->getKey()));
}

void DynamicThemeNode::onFiles(CCObject* sender) {
    utils::file::openFolder(Mod::get()->getConfigDir() / "themes");
}

void DynamicThemeNode::onDocs(CCObject* sender) {
    utils::web::openLinkInBrowser("https://bin.smjsproductions.com/smjs.gdintercept/pages");
}

$execute {
    (void) Mod::get()->registerCustomSettingType("dynamic-theme", &DynamicTheme::parse);
}