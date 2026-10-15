#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include "../include.hpp"
#include "codeblock/CodeBlock.hpp"
#include "capturelist/CaptureList.hpp"

class InterceptPopup : public Popup {
public:
    static InterceptPopup* get();
    static void scene();

    void reloadList();
    void reloadCodeBlock(const bool recycleInfo);
    void reloadCode();
    void reloadSideBar();
protected:
    bool init(CCSize size);
private:
    static CCSize POPUP_PADDING;
    static float UI_PADDING;
    static float CAPTURE_CELL_HEIGHT;
    static float CAPTURE_CELL_BADGE_HEIGHT;

    float m_captureCellWidth;
    float m_leftColumnXPosition;
    bool m_settingsShouldReload;
    CodeBlock* m_codeBlock;
    CaptureList* m_list;
    FLAlertLayer* m_settings;

    InterceptPopup(const CCSize& size);
    void preReload();
    void postReload();
    void setupList();
    void setupCodeBlock(const bool recycleInfo = false);
    float getPageHeight();
    float getComponentYPosition(const float offset, const float itemHeight);
    void onSettings(CCObject* obj);
};