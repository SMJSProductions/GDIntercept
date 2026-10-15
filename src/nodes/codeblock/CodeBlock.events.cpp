#include "CodeBlock.hpp"

using namespace SideBarCell;
using namespace proxy::prelude;

const std::vector<SideBarView> CodeBlock::VIEWS({
    { 'I', [](const auto& icons) { return icons.info; }, "Info", [](CodeBlock* block) { block->onInfo(); } },
    { 'B', [](const auto& icons) { return icons.body; }, "Body", [](CodeBlock* block) { block->onBody(); } },
    { 'Q', [](const auto& icons) { return icons.query; }, "Query", [](CodeBlock* block) { block->onQuery(); } },
    { 'H', [](const auto& icons) { return icons.header; }, "Request Headers", [](CodeBlock* block) { block->onRequestHeaders(); } },
    { 'R', [](const auto& icons) { return icons.response; }, "Response", [](CodeBlock* block) { block->onResponse(); } },
    { 'H', [](const auto& icons) { return icons.header; }, "Response Headers", [](CodeBlock* block) { block->onResponseHeaders(); } }
});

const SideBar::Categories CodeBlock::ACTIONS {
    { { "Save Files", "saves.png"_spr }, {
        { { "Open Save Files", "folder.png"_spr, [](CodeBlock* block) { return block->onOpenSaveFiles(); } } },
        { { "Save", "download.png"_spr, [](CodeBlock* block) { return block->onSave(); } }, [](CodeBlock* block) { return block->m_info != nullptr; } }
    } },
    { { "Current Request", "link.png"_spr }, {
        { { "Copy View", "copy.png"_spr, [](CodeBlock* block) { return block->onCopyView(); } }, [](CodeBlock* block) { return block->m_info != nullptr; } },
        { { "Copy Info", "copy.png"_spr, [](CodeBlock* block) { return block->onCopyInfo(); } }, [](CodeBlock* block) { return block->m_info != nullptr; } },
        { { "Copy Curl", "copy.png"_spr, [](CodeBlock* block) { return block->onCopyCurl(); } }, [](CodeBlock* block) { return block->m_info != nullptr; } },
        { { "Resend", "redo.png"_spr, [](CodeBlock* block) { return block->onSend(); } }, [](CodeBlock* block) { return block->m_info != nullptr; } },
        { { "Cancel", "cancel.png"_spr, [](CodeBlock* block) { return block->onCancel(); } }, [](CodeBlock* block) {
            return block->m_info && block->m_info->getClient() == Client::COCOS && (block->m_info->isInProgress() || block->m_info->isPaused());
        } }
    } },
    { {"Settings", "settings.png"_spr }, {
        {
            { "Resume Cocos", "resume.png"_spr, [](CodeBlock* block) { return block->onResume(); } },
            { "Pause Cocos", "pause.png"_spr, [](CodeBlock* block) { return block->onPause(); } },
            [](CodeBlock* block) { return ProxyHandler::isPaused(); }
        },
        { "raw-data", { "Raw", "Formatted" }, { "raw.png"_spr, "simplified.png"_spr }, false, {
            [](CodeBlock* block, const OriginalCallback& original) { return block->onRaw(original); },
            [](CodeBlock* block, const OriginalCallback& original) { return block->onFormatted(original); }
        } }
    } }
};

bool CodeBlock::ACCEPTED_PAUSES = false;

void CodeBlock::setup() {
    this->bind("open_save_files", [this]() {
        this->onOpenSaveFiles();
    });
    this->bind("save_packet", [this]() {
        this->onSave();
    });
    this->bind("copy_view", [this]() {
        this->onCopyView();
    });
    this->bind("copy_info", [this]() {
        this->onCopyInfo();
    });
    this->bind("copy_curl", [this]() {
        this->onCopyCurl();
    });
    this->bind("resend_packet", [this]() {
        this->onSend();
    });
    this->bind("pause_packet", [this]() {
        if (ProxyHandler::isPaused()) {
            this->onResume();
        } else {
            this->onPause();
        }
    });
    this->bind("raw_code_block", [this]() {
        Mod* mod = Mod::get();

        if (mod->getSettingValue<bool>("raw-data")) {
            this->onFormatted([mod]() {
                mod->setSettingValue("raw-data", false);

                return true;
            });
        } else {
            this->onRaw([mod]() {
                mod->setSettingValue("raw-data", true);

                return true;
            });
        }
    });

    this->bind("code_line_up", [this]() {
        const Theme::Theme& theme = Theme::getTheme();

        this->scroll(0, -(theme.code.font.getFontSize().height + theme.code.font.lineHeight));
    }, true);
    this->bind("code_line_down", [this]() {
        const Theme::Theme& theme = Theme::getTheme();

        this->scroll(0, theme.code.font.getFontSize().height + theme.code.font.lineHeight);
    }, true);
    this->bind("code_page_up", [this]() {
        this->scroll(0, -this->getNode()->getContentHeight());
    }, true);
    this->bind("code_page_down", [this]() {
        this->scroll(0, this->getNode()->getContentHeight());
    }, true);
    this->bind("code_page_left", [this]() {
        const CullingList* list = typeinfo_cast<CullingList*>(this->getNode());
        const std::vector<CullingCell*>& cells = list->getCells();

        this->scroll(cells.empty() ? 0 : list->getContentWidth() - reinterpret_cast<CodeSectionCell*>(cells.front())->getLineNumberWidth(), 0);
    }, true);
    this->bind("code_page_right", [this]() {
        const CullingList* list = typeinfo_cast<CullingList*>(this->getNode());
        const std::vector<CullingCell*>& cells = list->getCells();

        this->scroll(cells.empty() ? 0 : -(list->getContentWidth() - reinterpret_cast<CodeSectionCell*>(cells.front())->getLineNumberWidth()), 0);
    }, true);
}

bool CodeBlock::onCopyView() {
    utils::clipboard::write(m_code.zString());
    this->showMessage("View Copied");

    return true;
}

bool CodeBlock::onCopyInfo() {
    utils::clipboard::write(m_info->toString());
    this->showMessage("Info Copied");

    return true;
}

bool CodeBlock::onCopyCurl() {
    utils::clipboard::write(m_info->toCurl());
    this->showMessage("Command Copied");

    return true;
}

bool CodeBlock::onSend() {
    const HttpInfo::Request& original = m_info->getRequest();
    const std::string_view body = original.getBody();
    web::WebRequest request;

    if (body.size()) {
        request.bodyString(body);
    }

    for (const auto& [name, list] : original.getHeaders()) {
        for (const std::string& value : list) {
            request.header(name, std::move(value));
        }
    }

    m_resendTasks.emplace_back(async::spawn(request.send(original.getMethod(), m_info->getURL().getReconstruction())));

    this->showMessage("Request Resent");

    return true;
}

bool CodeBlock::onCancel() {
    m_info->cancel();
    this->showMessage("Request Cancelled");

    return true;
}

bool CodeBlock::onOpenSaveFiles() {
    Mod* mod = Mod::get();

    utils::file::openFolder(mod->getSavedValue("default-path", mod->getSaveDir()));

    return true;
}

bool CodeBlock::onSave() {
    Mod* mod = Mod::get();

    async::spawn(utils::file::pick(file::PickMode::OpenFolder, {
        .defaultPath = mod->getSavedValue("default-path", mod->getSaveDir())
    }), [this, mod](Result<std::optional<std::filesystem::path>> result) {
        Loader::get()->queueInMainThread([this, mod, result = std::move(result)] mutable {
            std::string filename = fmt::format("{:%Y-%m-%d_%H-%M-%S} - {}.txt", geode::localtime(std::time(nullptr)), m_info->getURL().getHost());

            if (result.isErr()) {
                return this->showMessage("Error Picking File", { 0xFF, 0x00, 0x00 });
            }

            std::optional<std::filesystem::path> optPah = std::move(result).unwrap();

            if (!optPah) {
                this->showMessage("File Save Cancelled", { 0xFF, 0xFF, 0x00 });
            }

            std::filesystem::path path = std::move(optPah).value();

            if (utils::file::writeString(path / std::move(filename), m_info->toString()).isOk()) {
                mod->setSavedValue("default-path", path);

                this->showMessage("File Saved");
            } else {
                this->showMessage("Error Saving File", { 0xFF, 0x00, 0x00 });
            }
        });
    });

    return true;
}

bool CodeBlock::onPause() {
    if (CodeBlock::ACCEPTED_PAUSES) {
        Warning::show();
        this->showMessage("Requests Paused");

        return true;
    } else {
        FLAlertLayer::create(this, "Pausing Requests",
            "<cr>All Cocos requests</c> will be <cy>paused</c> until you resume them. "
            "This means that you will no longer be able to send or receive online data from GD.\n\n"
            "Are you sure you want to <cy>pause</c> <cr>all Cocos requests</c>?"
        , "Cancel", "Ok")->show();

        return false;
    }
}

bool CodeBlock::onResume() {
    ProxyHandler::resumeAll();
    Warning::hide();
    this->showMessage("Requests Resumed");

    return true;
}

bool CodeBlock::onRaw(const SideBarCell::OriginalCallback& original) {
    this->showMessage("Raw Data Enabled");

    return original();
}

bool CodeBlock::onFormatted(const SideBarCell::OriginalCallback& original) {
    this->showMessage("Formatted Data Enabled");

    return original();
}

void CodeBlock::onInfo() {
    if (m_info) {
        this->setCode({ ContentType::UNKNOWN_CONTENT, m_info->toString(true) });
    } else {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    }
}

void CodeBlock::onBody() {
    if (m_info) {
        this->setCode(m_info->getRequest().getBodyContent(Mod::get()->getSettingValue<bool>("raw-data")));
    } else {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    }
}

void CodeBlock::onQuery() {
    if (m_info) {
        this->setCode({ ContentType::JSON, m_info->getURL().stringifyQuery(Mod::get()->getSettingValue<bool>("raw-data")) });
    } else {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    }
}

void CodeBlock::onRequestHeaders() {
    if (m_info) {
        this->setCode(m_info->getRequest().getHeaderContent(Mod::get()->getSettingValue<bool>("raw-data")));
    } else {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    }
}

void CodeBlock::onResponse() {
    if (!m_info || m_info->isInProgress() || m_info->isPaused()) {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    } else if (m_info->isCompleted()) {
        this->setCode(m_info->getResponse()->getBodyContent(Mod::get()->getSettingValue<bool>("raw-data")));
    } else {
        this->setCode({ ContentType::JSON, m_info->getResponse()->stringifyStatusCode() });
    }
}

void CodeBlock::onResponseHeaders() {
    if (!m_info || m_info->isInProgress() || m_info->isPaused()) {
        this->setCode({ ContentType::UNKNOWN_CONTENT, "" });
    } else if (m_info->isCompleted()) {
        this->setCode(m_info->getResponse()->getHeaderContent(Mod::get()->getSettingValue<bool>("raw-data")));
    } else {
        this->setCode({ ContentType::JSON, m_info->getResponse()->stringifyStatusCode() });
    }
}

void CodeBlock::FLAlert_Clicked(FLAlertLayer* alert, const bool state) {
    if (state) {
        CodeBlock::ACCEPTED_PAUSES = true;

        Warning::show();
        this->showMessage("Requests Paused");
        m_bar->reloadState();
    }
}