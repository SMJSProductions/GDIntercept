#include "TracklessScrollbar.hpp"

TracklessScrollbar* TracklessScrollbar::create(const bool vertical, const CCSize& size, CCScrollLayerExt* scrollLayer) {
    TracklessScrollbar* instance = new TracklessScrollbar(vertical);

    if (instance && instance->init(size, scrollLayer)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

TracklessScrollbar::TracklessScrollbar(const bool vertical) : Scrollbar(), m_vertical(vertical) { }

bool TracklessScrollbar::init(const CCSize& size, CCScrollLayerExt* scrollLayer) {
    ESCAPE_WHEN(!Scrollbar::init(scrollLayer), false);

    NineSlice* newThumb = NineSlice::createWithSpriteFrameName("d_largeSquare_01_001.png");

    this->getTrack()->removeFromParent();
    this->getThumb()->removeFromParent();
    this->setThumb(newThumb);
    this->addChild(newThumb);
    newThumb->setScale(0.01f);
    newThumb->setAnchorPoint(m_vertical ? TOP_LEFT : BOTTOM_RIGHT);
    this->setContentSize({ size.width, size.height });
    this->scheduleUpdate();

    return true;
}

float TracklessScrollbar::calculateContentPosition(const float targetSize, const float contentSize, const float scrollSize, const float thumbSize, const float touchLocation) {
    const float scrollableSize = scrollSize - thumbSize;
    ESCAPE_WHEN(!scrollableSize, 0);
    const float thumbPosition = std::clamp(touchLocation - thumbSize / 2, 0.0f, scrollableSize);

    return -std::max(contentSize - targetSize, 0.0f) * thumbPosition / scrollableSize;
}

float TracklessScrollbar::calculateThumbSize(const float targetSize, const float contentSize, const float scrollSize) {
    return std::clamp(targetSize / contentSize * scrollSize, 2.0f, scrollSize);
}

float TracklessScrollbar::calculateThumbLocation(const float targetSize, const float contentSize, const float scrollSize, const float thumbSize, const float contentPosition) {
    const float listDelta = std::max(contentSize - targetSize, 0.0f);
    ESCAPE_WHEN(!listDelta, scrollSize);
    const float thumbPercentage = std::clamp(contentPosition / listDelta + 1, 0.0f, 1.0f);

    return scrollSize - (scrollSize - thumbSize) * thumbPercentage;
}

void TracklessScrollbar::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    CCScrollLayerExt* target = this->getTarget();
    const CCSize thumbSize = this->getThumb()->getScaledContentSize();
    const CCPoint touchLocation = this->convertTouchToNodeSpace(touch);

    if (m_vertical) {
        target->m_contentLayer->setPositionY(this->calculateContentPosition(
            target->getContentHeight(),
            target->m_contentLayer->getContentHeight(),
            this->getContentHeight(),
            thumbSize.height,
            touchLocation.y
        ));
    } else {
        target->m_contentLayer->setPositionX(this->calculateContentPosition(
            target->getContentWidth(),
            target->m_contentLayer->getContentWidth(),
            this->getContentWidth(),
            thumbSize.width,
            touchLocation.x
        ));
    }
}

void TracklessScrollbar::update(const float dt) {
    CCScrollLayerExt* target = this->getTarget();
    NineSlice* thumb = this->getThumb();
    const Theme::Theme& theme = Theme::getTheme();
    const CCSize& size = this->getContentSize();
    const CCSize& targetSize = target->getContentSize();
    const CCSize& contentSize = target->m_contentLayer->getContentSize();
    const CCSize thumbSize = thumb->getScaledContentSize();

    if (this->isTouching()) {
        theme.code.scrollbar.activeThumb.applyTo(thumb);
    } else {
        theme.code.scrollbar.thumb.applyTo(thumb);
    }

    ESCAPE_WHEN(m_vertical ?
        contentSize.height == targetSize.height && targetSize.height == thumbSize.height :
        contentSize.width == targetSize.width && targetSize.width == thumbSize.width,);
    Scrollbar::update(dt);

    if (m_vertical) {
        thumb->setScaledContentSize({ size.width, this->calculateThumbSize(targetSize.height, contentSize.height, size.height) });
        thumb->setPositionY(this->calculateThumbLocation(
            targetSize.height, 
            contentSize.height, 
            size.height, 
            thumbSize.height, 
            target->m_contentLayer->getPositionY()
        ));
    } else {
        thumb->setScaledContentSize({ this->calculateThumbSize(targetSize.width, contentSize.width, size.width), size.height });
        thumb->setPositionX(this->calculateThumbLocation(
            targetSize.width,
            contentSize.width,
            size.width,
            thumbSize.width,
            target->m_contentLayer->getPositionX()
        ));
    }
}

void TracklessScrollbar::draw() {
    const CCSize size = this->getContentSize();

    ccDrawColor4B(Theme::getTheme().code.scrollbar.border);
    glLineWidth(2);

    if (m_vertical) {
        ccDrawLine(ZERO_POINT, { 0, size.height });
    } else {
        ccDrawLine({ 0, size.height }, size);
    }
}