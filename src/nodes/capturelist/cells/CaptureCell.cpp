#include "CaptureCell.hpp"

using namespace proxy::prelude;

CaptureCell* CaptureCell::create(const size_t index, std::shared_ptr<HttpInfo> info, const CCSize& size, const std::function<void(CaptureCell*)>& switchCell) {
    CaptureCell* instance = new CaptureCell(info, size, switchCell);

    if (instance && instance->init(index, size)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

template<>
std::tuple<std::string, CCNode*, std::function<void(GLubyte)>> CaptureCell::makeBadgeInfo(std::string name, CCSprite* sprite) {
    return { std::move(name), sprite, [sprite](GLubyte opacity) {
        sprite->setOpacity(opacity);
    } };
}

template<>
std::tuple<std::string, CCNode*, std::function<void(GLubyte)>> CaptureCell::makeBadgeInfo(std::string name, Character* character) {
    return { std::move(name), character, [character](GLubyte opacity) {
        character->setOpacity(opacity);
    } };
}

CaptureCell::CaptureCell(std::shared_ptr<HttpInfo> info, const CCSize& size, const std::function<void(CaptureCell*)>& switchCell) : GenericListCell("", size),
m_info(info),
m_switchCell(switchCell) { }

bool CaptureCell::init(const size_t index, const CCSize& size) {
    const float menuScale = 0.3f;
    const float menuPositionX = size.width - 20;
    const bool hideBadges = Mod::get()->getSettingValue<bool>("hide-badges");
    const HttpInfo::Request& request = m_info->getRequest();
    const URL& url = m_info->getURL();
    const std::string& path = url.getPath();
    std::string requestLabel = m_info->getRequest().getMethod();

    requestLabel.push_back(' ');
    requestLabel.append(path == "/" ? "" : path.substr(path.substr(0, path.size() - 1).find_last_of('/')));

    if (requestLabel.back() == ' ') {
        requestLabel.append(url.getHost());
    }

    CCLabelBMFont* label = CCLabelBMFont::create(requestLabel.c_str(), "bigFont.fnt");
    ButtonSprite* button = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png");
    CCMenu* menu = CCMenu::createWithItem(CCMenuItemSpriteExtra::create(
        button,
        this,
        menu_selector(CaptureCell::onView)
    ));
    const float maxContentWidth = menuPositionX - button->getContentWidth() * menuScale / 2 - PADDING * 2;

    menu->setAnchorPoint(BOTTOM_LEFT);
    menu->setPosition({ menuPositionX, size.height / 2 });
    menu->setScale(menuScale);
    button->setID("view"_spr);
    label->setAnchorPoint(CENTER_LEFT);
    label->setPositionX(PADDING);
    label->limitLabelWidth(maxContentWidth, 0.2f, 0.05f);

    if (hideBadges) {
        label->setPositionY(size.height / 2);
    } else {
        const auto badges = this->badgesForRequest();
        CCScale9Sprite* badgesBg = CCScale9Sprite::create("square02b_001.png");

        label->setPositionY(size.height - PADDING);
        badgesBg->setPosition({ PADDING, PADDING });
        badgesBg->setScale(0.25f);
        badgesBg->setScaledContentSize({ maxContentWidth, label->getPositionY() - PADDING * 2 });
        badgesBg->setColor(index % 2 == 0 ? LIGHTER_BROWN_3B : LIGHT_BROWN_3B);
        badgesBg->setAnchorPoint(BOTTOM_LEFT);

        for (size_t i = 0; i < badges.size(); i++) {
            const auto& [ name, sprite, callback ] = badges.at(i);
            RescalingNode* badgeNode = RescalingNode::create(sprite, { 40, 40 });
            Badge* badge = Badge::create(name, badgeNode, callback);

            badgeNode->setRescaleWidth(false);
            badge->setAnchorPoint(CENTER);
            badge->setPosition({
                PADDING * 2 + 24 + 48 * i,
                badgesBg->getContentSize().height / 2
            });
            badgesBg->addChild(badge);
        }

        m_mainLayer->addChild(badgesBg);
    }

    m_mainLayer->addChild(label);
    m_mainLayer->addChild(menu);

    for (size_t i = 0; i < request.getMethod().size(); i++) {
        label->getChildByIndex<CCSprite>(i)->setColor(this->colorForMethod());
    }

    return true;
}

std::vector<std::tuple<std::string, CCNode*, std::function<void(GLubyte)>>> CaptureCell::badgesForRequest() {
    std::vector<std::tuple<std::string, CCNode*, std::function<void(GLubyte)>>> badges;

    switch (m_info->getState()) {
        case State::PAUSED: badges.push_back(
            this->makeBadgeInfo("Paused", CCSprite::createWithSpriteFrameName("particle_194_001.png"))
        ); break;
        case State::IN_PROGRESS: badges.push_back(
            this->makeBadgeInfo("In Progress", CCSprite::createWithSpriteFrameName("particle_197_001.png"))
        ); break;
        case State::COMPLETED: {
            const int status = m_info->getResponse()->getStatusCode();

            if (status >= 300 && status < 400) {
                CCSprite* sprite = CCSprite::createWithSpriteFrameName("particle_12_001.png");

                sprite->setColor({ 0xFF, 0x96, 0x4F });
                badges.push_back(this->makeBadgeInfo("Redirected", sprite));
            } else if (status >= 400 && status < 500) {
                CCSprite* sprite = CCSprite::createWithSpriteFrameName("particle_203_001.png");

                sprite->setColor({ 0xF0, 0x1E, 0x2C });
                badges.push_back(this->makeBadgeInfo("Client Error", sprite));
            } else if (status >= 500 && status < 600) {
                CCSprite* sprite = CCSprite::createWithSpriteFrameName("particle_10_001.png");

                sprite->setColor({ 0xF0, 0x1E, 0x2C });
                badges.push_back(this->makeBadgeInfo("Server Error", sprite));
            } else {
                badges.push_back(this->makeBadgeInfo("Completed", CCSprite::createWithSpriteFrameName("particle_50_001.png")));
            }
        } break;
        case State::FAULTY: badges.push_back(
            this->makeBadgeInfo("Faulty", CCSprite::createWithSpriteFrameName("info-alert.png"_spr))
        ); break;
        case State::CANCELLED: badges.push_back(
            this->makeBadgeInfo("Cancelled", CCSprite::createWithSpriteFrameName("particle_193_001.png"))
        ); break;
    }

    switch (m_info->getURL().getOrigin()) {
        case Origin::GD: badges.push_back(
            this->makeBadgeInfo("Geometry Dash Server Origin", CCSprite::createWithSpriteFrameName("gd.png"_spr))
        ); break;
        case Origin::GD_CDN: badges.push_back(
            this->makeBadgeInfo("Geometry Dash CDN Origin", CCSprite::createWithSpriteFrameName("particle_202_001.png"))
        ); break;
        case Origin::ROBTOP_GAMES: badges.push_back(
            this->makeBadgeInfo("Robtop Games Server Origin", Character::create('R', "goldFont.fnt", true))
        ); break;
        case Origin::NEWGROUNDS: badges.push_back(
            this->makeBadgeInfo("Newgrounds Server Origin", CCSprite::createWithSpriteFrameName("ng.png"_spr))
        ); break;
        case Origin::GEODE: badges.push_back(
            this->makeBadgeInfo("Geode Server Origin", CCSprite::createWithSpriteFrameName("geode-circle.png"_spr))
        ); break;
        case Origin::LOCALHOST: badges.push_back(
            this->makeBadgeInfo("Localhost Origin", CCSprite::createWithSpriteFrameName("pc.png"_spr))
        ); break;
        case Origin::LAN: badges.push_back(
            this->makeBadgeInfo("LAN", CCSprite::createWithSpriteFrameName("globe.png"_spr))
        ); break;
        case Origin::OTHER: badges.push_back(
            this->makeBadgeInfo("Unknown Origin", Character::create('?', "goldFont.fnt", true))
        ); break;
    }

    if (m_info->isRepeat()) {
        badges.push_back(this->makeBadgeInfo("Request Repeat", CCSprite::createWithSpriteFrameName("reset-gold.png"_spr)));
    }

    return badges;

}

ccColor3B CaptureCell::colorForMethod() {
    const std::string& method = m_info->getRequest().getMethod();

    if (method == "GET") {
        return { 0xA8, 0x96, 0xFF };
    } else if (method == "POST") {
        return { 0x7E, 0xCF, 0x2B };
    } else if (method == "PUT") {
        return { 0xFF, 0x9A, 0x1F };
    } else if (method == "DELETE") {
        return { 0xFF, 0x56, 0x31 };
    } else {
        return { 0x46, 0xC1, 0xE6 };
    }
}

void CaptureCell::activate() {
    m_switchCell(this);
    typeinfo_cast<ButtonSprite*>(m_mainLayer->getChildByIDRecursive("view"_spr))->updateBGImage("GJ_button_05.png");
}

void CaptureCell::deactivate() {
    typeinfo_cast<ButtonSprite*>(m_mainLayer->getChildByIDRecursive("view"_spr))->updateBGImage("GJ_button_01.png");
}

void CaptureCell::onView(CCObject* obj) {
    this->activate();
}