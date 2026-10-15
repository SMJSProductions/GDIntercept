#include "ActionCell.hpp"

using namespace SideBarCell;

SideBarActionButton::SideBarActionButton() : name(""),
icon("") { }

SideBarActionButton::SideBarActionButton(std::string name, std::string icon, const StandardCallback& callback) : name(std::move(name)),
icon(std::move(icon)),
callback(callback) { }

bool SideBarActionButton::isEmpty() const {
    return name.empty() || icon.empty();
}

SideBarAction::SideBarAction(const SideBarActionButton& on, const StandardCallback& enableState) : on(on), enableState(enableState) { }

SideBarAction::SideBarAction(
    const SideBarActionButton& on,
    const SideBarActionButton& off,
    const StandardCallback& determineState,
    const StandardCallback& enableState
) : on(on),
off(off),
determineState(determineState),
enableState(enableState) { }

SideBarAction::SideBarAction(
    const std::string& setting,
    const std::tuple<std::string, std::string>& names,
    const std::tuple<std::string, std::string>& icons,
    const bool inverse,
    const std::tuple<OverrideCallback, OverrideCallback>& overrides
) : on({
    std::get<0>(names),
    std::get<0>(icons),
    [setting, inverse, overrides](CodeBlock* block) {
        const auto& override = inverse ? std::get<1>(overrides) : std::get<0>(overrides);
        const auto callback = [setting, inverse]{ Mod::get()->setSettingValue(setting, !inverse); return true; };

        if (override) {
            return override(block, callback);
        } else {
            return callback();
        }
    }
}),
off({
    std::get<1>(names),
    std::get<1>(icons),
    [setting, inverse, overrides](CodeBlock* block) {
        const auto& override = inverse ? std::get<0>(overrides) : std::get<1>(overrides);
        const auto callback = [setting, inverse]{ Mod::get()->setSettingValue(setting, inverse); return true; };

        if (override) {
            return override(block, callback);
        } else {
            return callback();
        }
    }
}),
determineState([setting, inverse](CodeBlock* block) {
    return Mod::get()->getSettingValue<bool>(setting) == inverse;
}) { }

ActionCell* SideBarAction::toActionCell(CodeBlock* block) const {
    return ActionCell::create(block, *this);
}

ActionCell* ActionCell::create(CodeBlock* block, const SideBarAction& action) {
    ActionCell* instance = new ActionCell(block, action);

    if (instance && instance->init()) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

ActionCell::ActionCell(CodeBlock* block, const SideBarAction& action) : m_state(action.off.isEmpty() || action.determineState(block)),
m_action(action),
m_block(block),
m_enabled(true) { }

bool ActionCell::init() {
    const Theme::Theme& theme = Theme::getTheme();

    ESCAPE_WHEN(!HoverNode::init(), false);
    ESCAPE_WHEN(!CCLayerColor::initWithColor(theme.menu.foreground), false);

    const float referenceHeight = theme.menu.font.getTrueFontSize().height;
    const bool oldState = m_state;
    SimpleAxisLayout* layout = SimpleRowLayout::create()
        ->setGap(theme.menu.paddingCenter)
        ->setPadding({ theme.menu.paddingLeft, 0, theme.menu.paddingRight, 0 })
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Between);

    m_icon = RescalingNode::create(CCNode::create(), { referenceHeight, referenceHeight });
    m_name = CenterLabel::create("0", theme.menu.font.fontName);

    theme.menu.text.applyTo(m_name);
    m_name->setScale(theme.menu.font.fontScale);

    this->setOpacity(0);
    this->setContentHeight(m_name->getScaledContentHeight() + theme.menu.font.lineHeight);
    this->addChild(m_icon);
    this->setLayout(layout);

    m_minWidth = this->getContentWidth();

    this->insertBefore(m_name, m_icon);
    this->setState(!oldState);

    m_maxWidth = this->getContentWidth();

    this->setState(oldState);

    m_maxWidth = std::max(m_maxWidth, this->getContentWidth());

    this->setLayout(layout->setMainAxisScaling(AxisScaling::None));

    return true;
}

void ActionCell::setState(const bool state) {
    m_state = state || m_action.off.isEmpty();
    const Theme::Theme& theme = Theme::getTheme();
    const SideBarActionButton button = m_state ? m_action.on : m_action.off;
    CCSprite* icon = CCSprite::createWithSpriteFrameName(button.icon.c_str());

    m_name->setCString(button.name.c_str());
    m_icon->setNode(icon);
    theme.menu.icons.actionIcon.applyTo(icon);
    this->updateLayout();
}

void ActionCell::hideOpacity(const float dt) {
    // Reset the hover state to reinitialize the opacity if needed
    m_hovered = false;

    this->setOpacity(0);
}

void ActionCell::enable() {
    const Theme::Theme& theme = Theme::getTheme();
    CCSprite* icon = typeinfo_cast<CCSprite*>(m_icon->getNode());

    m_enabled = true;

    theme.menu.icons.actionIcon.applyTo(icon);
    theme.menu.text.applyTo(m_name);
}

void ActionCell::disable() {
    const Theme::Theme& theme = Theme::getTheme();
    CCSprite* icon = typeinfo_cast<CCSprite*>(m_icon->getNode());

    m_enabled = false;

    theme.menu.icons.disabledIcon.applyTo(icon);
    theme.menu.disabledText.applyTo(m_name);
}

void ActionCell::onHover() {
    if (this->getOpacity() != 255 && m_enabled) {
        this->setOpacity(160);
    }
}

void ActionCell::unHover() {
    if (this->getOpacity() != 255) {
        this->setOpacity(0);
    }
}

void ActionCell::onScaleToMin() {
    m_name->setVisible(false);
    this->updateLayout();
}

void ActionCell::onScaleToMax() {
    m_name->setVisible(true);
    this->updateLayout();
}