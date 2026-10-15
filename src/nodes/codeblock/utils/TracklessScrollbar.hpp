#pragma once

#include "../../../include.hpp"

class TracklessScrollbar : public Scrollbar {
public:
    static TracklessScrollbar* create(const bool vertical, const CCSize& size, CCScrollLayerExt* scrollLayer);
private:
    const bool m_vertical;

    TracklessScrollbar(const bool vertical);
    bool init(const CCSize& size, CCScrollLayerExt* scrollLayer);
    float calculateContentPosition(const float targetSize, const float contentSize, const float scrollSize, const float thumbSize, const float touchLocation);
    float calculateThumbSize(const float targetSize, const float contentSize, const float scrollSize);
    float calculateThumbLocation(const float targetSize, const float contentSize, const float scrollSize, const float thumbSize, const float contentPosition);
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void update(const float dt) override;
    void draw() override;
};