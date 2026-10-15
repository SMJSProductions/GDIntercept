#include "CenterLabel.hpp"

CenterLabel* CenterLabel::create(const ZStringView string, const ZStringView font) {
    CenterLabel* instance = new CenterLabel();

    if (instance && instance->initWithString(string.c_str(), font.c_str())) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

void CenterLabel::updateLabel() {
    CCLabelBMFont::updateLabel();
    const size_t count = this->getChildrenCount();

    if (count) {
        CCSprite* referenceSprite = this->getChildByIndex<CCSprite>(0);
        const float referenceHeight = CCLabelBMFont::create("0", this->getFntFile())->getContentHeight() * this->getScale();
        const float heightAddition = referenceHeight / 2 - referenceSprite->getPositionY();

        for (int i = 0; i < count; i++) {
            CCNode* child = this->getChildByIndex(i);

            child->setPositionY(child->getPositionY() + heightAddition);
        }
    }
}