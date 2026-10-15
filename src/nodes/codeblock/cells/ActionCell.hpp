#pragma once

#include "../../../include.hpp"
#include "../../utils/HoverNode.hpp"
#include "../../utils/CenterLabel.hpp"
#include "../utils/SharedWidthNode.hpp"
#include "../../utils/RescalingNode.hpp"

class CodeBlock;
class ActionCell;

namespace SideBarCell {
    using OriginalCallback = std::function<bool()>;
    using StandardCallback = std::function<bool(CodeBlock* block)>;
    using OverrideCallback = std::function<bool(CodeBlock* block, const OriginalCallback& original)>;

    struct SideBarActionButton {
        const std::string name;
        const std::string icon;
        const StandardCallback callback;

        SideBarActionButton();
        SideBarActionButton(std::string name, std::string icon, const StandardCallback& callback);
        bool isEmpty() const;
    };

    struct SideBarAction {
        const SideBarActionButton on;
        const SideBarActionButton off;
        const StandardCallback determineState;
        const StandardCallback enableState;

        SideBarAction(const SideBarActionButton& on, const StandardCallback& enableState = {});
        SideBarAction(const SideBarActionButton& on, const SideBarActionButton& off, const StandardCallback& determineState, const StandardCallback& enableState = {});
        SideBarAction(
            const std::string& setting,
            const std::tuple<std::string, std::string>& names,
            const std::tuple<std::string, std::string>& icons,
            const bool inverse,
            const std::tuple<OverrideCallback, OverrideCallback>& overrides = {}
        );
        ActionCell* toActionCell(CodeBlock* block) const;
    };
}

class ActionCell : public SharedWidthNode<HoverNode<CCLayerColor>> {
public:
    static ActionCell* create(CodeBlock* block, const SideBarCell::SideBarAction& action);

    void setState(const bool state);
    void hideOpacity(const float dt);
    void enable();
    void disable();
private:
    const SideBarCell::SideBarAction m_action;
    RescalingNode* m_icon;
    CCLabelBMFont* m_name;
    CodeBlock* m_block;
    BOOL_GETTER(enabled, Enabled);
    BOOL_GETTER(state, State);

    ActionCell(CodeBlock* block, const SideBarCell::SideBarAction& action);
    bool init() override;
    void onHover() override;
    void unHover() override;
    void onScaleToMin() override;
    void onScaleToMax() override;
};