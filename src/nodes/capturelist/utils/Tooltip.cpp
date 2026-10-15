#include "Tooltip.hpp"

Tooltip* Tooltip::create(const ZStringView text, const float scale, const GLubyte opacity) {
    Tooltip* instance = new Tooltip();

    if (instance && instance->init(text, scale, opacity)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

bool Tooltip::init(const ZStringView text, const float scale, const GLubyte opacity) {
    CCLabelBMFont* label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");

    label->setScale(scale);

    const CCSize labelSize = label->getScaledContentSize() + ccp(PADDING, PADDING) * 2;

    ESCAPE_WHEN(!Popup::init(labelSize.width, labelSize.height, "square02_001.png"), false);

    this->ignoreAnchorPointForPosition(false);
    this->setContentSize(m_size);
    this->setOpacity(0);
    m_mainLayer->setPosition(m_size * m_mainLayer->getAnchorPoint());
    m_bgSprite->setScale(0.2f);
    m_bgSprite->setOpacity(opacity);
    m_bgSprite->setScaledContentSize(m_size);
    m_bgSprite->setPosition(m_size * m_bgSprite->getAnchorPoint());
    label->setPosition(this->getContentSize() / 2);
    m_mainLayer->addChild(label);
    m_buttonMenu->removeFromParent();

    return true;
}