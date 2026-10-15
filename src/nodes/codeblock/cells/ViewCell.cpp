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

    SimpleAxisLayout* layout = SimpleRowLayout::create()
        ->setGap(theme.menu.paddingCenter)
        ->setPadding({ theme.menu.paddingLeft, 0, theme.menu.paddingRight, 0 })
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Start);

    m_icon = Character::create(view.icon, theme.menu.font.fontName);
    m_name = CenterLabel::create(view.name, theme.menu.font.fontName);

    m_icon->setScale(theme.menu.font.fontScale);
    m_name->setScale(theme.menu.font.fontScale);
    view.iconColor(theme.menu.icons).applyTo(m_icon);
    theme.menu.text.applyTo(m_name);

    this->setOpacity(0);
    this->setContentHeight(m_name->getScaledContentHeight() + theme.menu.font.lineHeight);
    this->addChild(m_icon);
    this->setLayout(layout);

    m_minWidth = this->getContentWidth();
    m_maxWidth = m_minWidth + theme.menu.paddingCenter + m_name->getScaledContentWidth();

    this->addChild(m_name);
    this->setLayout(layout->setMainAxisScaling(AxisScaling::None));

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
    this->updateLayout();
}

void ViewCell::onScaleToMax() {
    m_name->setVisible(true);
    this->updateLayout();
}