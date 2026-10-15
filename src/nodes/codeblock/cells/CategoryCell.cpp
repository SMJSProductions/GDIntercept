#include "CategoryCell.hpp"

using namespace SideBarCell;

bool SideBarCategory::operator==(const SideBarCategory& other) const {
    return icon == other.icon && name == other.name;
}

CategoryCell* SideBarCategory::toCategoryCell() const {
    return CategoryCell::create(*this);
}

CategoryCell* CategoryCell::create(const SideBarCategory& view) {
    CategoryCell* instance = new CategoryCell();

    if (instance && instance->init(view)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

bool CategoryCell::init(const SideBarCategory& view) {
    const Theme::Theme& theme = Theme::getTheme();

    ESCAPE_WHEN(!CCLayerColor::initWithColor(theme.menu.categoryBackground), false);
    ESCAPE_WHEN(!SharedWidthNode::init(), false);

    const float referenceHeight = theme.menu.font.getTrueFontSize().height;
    CCSprite* icon = CCSprite::createWithSpriteFrameName(view.icon.c_str());
    SimpleAxisLayout* layout = SimpleRowLayout::create()
        ->setGap(theme.menu.paddingCenter)
        ->setPadding({ theme.menu.paddingLeft, 0, theme.menu.paddingRight, 0 })
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Between);

    m_icon = RescalingNode::create(icon, { referenceHeight, referenceHeight });
    m_name = CenterLabel::create(view.name, theme.menu.font.fontName);

    m_name->setScale(theme.menu.font.fontScale);
    theme.menu.icons.categoryIcon.applyTo(icon);
    theme.menu.categoryText.applyTo(m_name);

    this->setContentHeight(m_name->getScaledContentHeight() + theme.menu.font.lineHeight);
    this->addChild(m_icon);
    this->setLayout(layout);

    m_minWidth = this->getContentWidth();
    m_maxWidth = m_minWidth + theme.menu.paddingCenter + m_name->getScaledContentWidth();

    this->insertBefore(m_name, m_icon);
    this->setLayout(layout->setMainAxisScaling(AxisScaling::None));

    return true;
}

void CategoryCell::onHover() {
    // Do nothing
}

void CategoryCell::onScaleToMin() {
    m_name->setVisible(false);
    this->updateLayout();
}

void CategoryCell::onScaleToMax() {
    m_name->setVisible(true);
    this->updateLayout();
}

void CategoryCell::draw() {
    const CCSize& size = this->getContentSize();

    SharedWidthNode::draw();

    ccDrawColor4B(Theme::getTheme().menu.categoryBorder);
    ccDrawLine(ZERO_POINT, { size.width, 0 });
    ccDrawLine({ 0, size.height }, size);
}