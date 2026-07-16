#pragma once

#include "../../../include.hpp"
#include "../../utils/Character.hpp"
#include "../../utils/HoverNode.hpp"
#include "../../utils/CenterLabel.hpp"
#include "../utils/SharedWidthNode.hpp"

class CodeBlock;
class ViewCell;

namespace SideBarCell {
    struct SideBarView {
        const char icon;
        const std::function<Color(const Theme::Menu::Icons& icons)> iconColor;
        const std::string name;
        const std::function<void(CodeBlock* block)> callback;

        ViewCell* toViewCell() const;
    };
}

class ViewCell : public SharedWidthNode<HoverNode<CCLayerColor>> {
public:
    static ViewCell* create(const SideBarCell::SideBarView& view);
private:
    Character* m_icon;
    CCLabelBMFont* m_name;

    bool init(const SideBarCell::SideBarView& view);
    void onHover() override;
    void unHover() override;
    void onScaleToMin() override;
    void onScaleToMax() override;
};