#include "InterceptPopup.hpp"

using namespace proxy::prelude;

CCSize InterceptPopup::POPUP_PADDING = { 60, 40 };

float InterceptPopup::UI_PADDING = 2 + PADDING;

float InterceptPopup::CAPTURE_CELL_HEIGHT = 20;

float InterceptPopup::CAPTURE_CELL_BADGE_HEIGHT = 10;

InterceptPopup* InterceptPopup::get() {
    CCScene* scene = CCDirector::sharedDirector()->getRunningScene();

    if (scene) {
        return typeinfo_cast<InterceptPopup*>(scene->getChildByID("intercept_popup"_spr));
    } else {
        return nullptr;
    }
}

void InterceptPopup::scene() {
    CCSize uiSize = CCDirector::sharedDirector()->getWinSize() - InterceptPopup::POPUP_PADDING;

    if (!InterceptPopup::get()) {
        InterceptPopup* instance = new InterceptPopup(uiSize);

        if (instance && instance->init(std::move(uiSize))) {
            instance->m_noElasticity = true;
            instance->setID("intercept_popup"_spr);
            instance->autorelease();
            instance->show();
        } else {
            delete instance;
        }
    }
}

InterceptPopup::InterceptPopup(const CCSize& size) : m_captureCellWidth(size.width / 3),
m_leftColumnXPosition(InterceptPopup::UI_PADDING + PADDING + m_captureCellWidth),
m_settingsShouldReload(false),
m_codeBlock(nullptr),
m_list(nullptr),
m_settings(nullptr) { }

bool InterceptPopup::init(CCSize size) {
    ESCAPE_WHEN(!Popup::init(std::move(size)), false);

    this->setTitle("Intercepted Requests");
    this->setupCodeBlock();
    this->setupList();

    CCSprite* settingsSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    CCMenuItemSpriteExtra* item = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(InterceptPopup::onSettings));

    settingsSprite->setScale(0.4f);
    settingsSprite->setPosition(settingsSprite->getScaledContentSize() / 2);
    item->setContentSize(settingsSprite->getScaledContentSize());
    item->setPosition({
        m_size.width - InterceptPopup::UI_PADDING - settingsSprite->getContentWidth() * settingsSprite->getScale() / 2,
        m_title->getPositionY()
    });
    m_buttonMenu->addChild(item);

    return true;
}

void InterceptPopup::reloadList() {
    OPT(m_list)->removeFromParent();

    this->preReload();
    this->setupList();
    this->postReload();
}

void InterceptPopup::reloadCodeBlock(const bool recycleInfo) {
    OPT(m_codeBlock)->removeFromParent();

    this->preReload();
    this->setupCodeBlock(recycleInfo);
    this->postReload();
}

void InterceptPopup::reloadCode() {
    m_codeBlock->reloadCode();
}

void InterceptPopup::reloadSideBar() {
    m_codeBlock->reloadSideBar();
}

void InterceptPopup::preReload() {
    CCScene* currentScene = CCDirector::sharedDirector()->getRunningScene();
    CCArrayExt<CCNode*> nodes = CCArrayExt<CCNode*>(currentScene->getChildren());

    if (std::find(nodes.begin(), nodes.end(), m_settings) != nodes.end()) {
        m_settings->removeFromParent();

        m_settingsShouldReload = true;
    }
}

void InterceptPopup::postReload() {
    if (m_settingsShouldReload) {
        m_settingsShouldReload = false;

        this->onSettings(nullptr);
    }
}

void InterceptPopup::setupList() {
    const float cellHeight = InterceptPopup::CAPTURE_CELL_HEIGHT + InterceptPopup::CAPTURE_CELL_BADGE_HEIGHT * !Mod::get()->getSettingValue<bool>("hide-badges");

    m_list = CaptureList::create({
        m_captureCellWidth,
        this->getPageHeight()
    }, cellHeight, [this](std::shared_ptr<HttpInfo> info) {
        m_codeBlock->updateInfo(info);
    });

    m_list->setAnchorPoint(BOTTOM_LEFT);
    m_list->setPosition({ InterceptPopup::UI_PADDING, InterceptPopup::UI_PADDING });
    m_mainLayer->addChild(m_list);
}

void InterceptPopup::setupCodeBlock(const bool recycleInfo) {
    std::shared_ptr<HttpInfo> info = recycleInfo ? m_codeBlock->getActiveInfo() : nullptr;

    m_codeBlock = CodeBlock::create({
        m_size.width - m_leftColumnXPosition - InterceptPopup::UI_PADDING,
        this->getPageHeight()
    });

    if (recycleInfo) {
        m_codeBlock->updateInfo(info);
    }

    m_codeBlock->setPosition({ m_leftColumnXPosition, InterceptPopup::UI_PADDING });
    m_mainLayer->addChild(m_codeBlock);
}

float InterceptPopup::getPageHeight() {
    return m_title->getPositionY() - m_title->getContentHeight() / 2 - InterceptPopup::UI_PADDING;
}

float InterceptPopup::getComponentYPosition(const float offset, const float itemHeight) {
    return InterceptPopup::UI_PADDING + this->getPageHeight() - itemHeight - offset;
}

void InterceptPopup::onSettings(CCObject* obj) {
    CCScene* currentScene = CCDirector::sharedDirector()->getRunningScene();

    openSettingsPopup(Mod::get());

    // If this breaks, someone should stop messing with addChild, that's a whole lot of not my problem
    m_settings = typeinfo_cast<FLAlertLayer*>(currentScene->getChildren()->objectAtIndex(currentScene->getChildrenCount() - 1));
}