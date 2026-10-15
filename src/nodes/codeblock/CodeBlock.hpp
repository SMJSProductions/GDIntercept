#pragma once

#include "../../include.hpp"
#include "SideBar.hpp"
#include "../Warning.hpp"
#include "cells/ViewCell.hpp"
#include "cells/ActionCell.hpp"
#include "cells/CategoryCell.hpp"
#include "utils/CullingList.hpp"
#include "cells/CullingCell.hpp"
#include "../utils/KeybindNode.hpp"
#include "cells/CodeSectionCell.hpp"
#include "utils/TracklessScrollbar.hpp"
#include "../../proxy/ProxyHandler.hpp"
#include "../../objects/JSONTokenizer2.hpp"

class CodeBlock : public KeybindNode<Border>, public FLAlertLayerProtocol {
public:
    static CodeBlock* create(const CCSize& size);

    void reloadCode();
    void reloadSideBar();
    void updateInfo(std::shared_ptr<proxy::HttpInfo> info);
    void setCode(proxy::HttpInfo::Content code);
    std::shared_ptr<proxy::HttpInfo> getActiveInfo() const;
private:
    static const std::vector<SideBarCell::SideBarView> VIEWS;
    static const SideBar::Categories ACTIONS;
    static bool ACCEPTED_PAUSES;

    std::shared_ptr<proxy::HttpInfo> m_info;
    std::vector<arc::TaskHandle<web::WebResponse>> m_resendTasks;
    SideBar* m_bar;
    UniqueString m_code;
    std::tuple<TracklessScrollbar*, TracklessScrollbar*> m_scrollbars;
    CCLayerColor* m_corner;

    CodeBlock();
    void setup() override;
    bool init(const CCSize& size);
    void setupSections(proxy::enums::ContentType contentType);
    void setupScrollbars();
    void showMessage(std::string message, const ccColor3B& color = { 0xFF, 0xFF, 0xFF });
    void scroll(const float x, const float y);
    bool onCopyView();
    bool onCopyInfo();
    bool onCopyCurl();
    bool onSend();
    bool onCancel();
    bool onOpenSaveFiles();
    bool onSave();
    bool onPause();
    bool onResume();
    bool onRaw(const SideBarCell::OriginalCallback& original);
    bool onFormatted(const SideBarCell::OriginalCallback& original);
    void onInfo();
    void onBody();
    void onQuery();
    void onRequestHeaders();
    void onResponse();
    void onResponseHeaders();
    void resizeList();
    void FLAlert_Clicked(FLAlertLayer* alert, const bool state) override;
};