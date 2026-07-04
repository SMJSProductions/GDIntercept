#include "ViewCell.hpp"

using namespace SideBarCell;

ViewCell* SideBarView::toViewCell() const {
    return ViewCell::create(*this);
}

ViewCell* ViewCell::create(const SideBarView& view) {
    ViewCell* instance = new ViewCell();

    if (instance && instance->init(view)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

bool ViewCell::init(const SideBarView& view) {
    const Theme::Theme& theme = Theme::getTheme();

    ESCAPE_WHEN(!HoverNode<CCLayerColor>::init(), false);
    ESCAPE_WHEN(!CCLayerColor::initWithColor(theme.menu.foreground), false);

    m_icon = Character::create(view.icon, theme.menu.font.fontName);

    m_icon->setScale(theme.menu.font.fontScale);

    const CCSize iconSize = m_icon->getScaledContentSize();
    const float height = iconSize.height + theme.menu.font.lineHeight;
    m_name = CenterLabel::create(view.name, theme.menu.font.fontName);

    view.iconColor(theme.menu.icons).applyTo(m_icon);
    m_icon->setAnchorPoint(CENTER_LEFT);
    m_icon->setPositionY(height / 2);
    theme.menu.text.applyTo(m_name);
    m_name->setScale(theme.menu.font.fontScale);
    m_name->setAnchorPoint(CENTER_LEFT);
    m_name->setPosition({ iconSize.width + theme.menu.paddingLeft + theme.menu.paddingCenter, height / 2 });

    this->setOpacity(0);
    this->setContentHeight(height);
    this->addChild(m_icon);
    this->addChild(m_name);

    m_minWidth = iconSize.width + theme.menu.paddingLeft + theme.menu.paddingRight;
    m_maxWidth = m_name->getScaledContentWidth() + m_name->getPositionX() + theme.menu.paddingRight;

    return true;
}

void ViewCell::onHover() {
    if (this->getOpacity() != 255) {
        this->setOpacity(160);
    }
}

void ViewCell::unHover() {
    if (this->getOpacity() != 255) {
        this->setOpacity(0);
    }
}

void ViewCell::onScaleToMin() {
    m_name->setVisible(false);
    m_icon->setAnchorPoint(CENTER);
    m_icon->setPositionX(m_minWidth / 2);
}

void ViewCell::onScaleToMax() {
    m_name->setVisible(true);
    m_icon->setAnchorPoint(CENTER_LEFT);
    m_icon->setPositionX(Theme::getTheme().menu.paddingLeft);
}