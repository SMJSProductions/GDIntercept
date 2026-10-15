#include "MenuLayer.hpp"

using namespace proxy::prelude;

bool ModMenuLayer::init() {
    ESCAPE_WHEN(!MenuLayer::init(), false);

    CCSprite* logo = CCSprite::create("GDI-round.png"_spr);

    logo->setScale(0.4f);

    CCNode* closeMenu = this->getChildByID("close-menu");
    CCMenuItemSpriteExtra* menuItem = CCMenuItemSpriteExtra::create(logo, this, menu_selector(ModMenuLayer::onLogoClick));

    if (!closeMenu) {
        closeMenu = this->getChildByID("top-right-menu");
    }

    if (ProxyHandler::isPaused()) {
        Warning::show();
    }

    menuItem->setID("GDI"_spr);
    closeMenu->addChild(menuItem);
    closeMenu->updateLayout();

    return true;
}

void ModMenuLayer::onLogoClick(CCObject* sender) {
    InterceptPopup::scene();
}