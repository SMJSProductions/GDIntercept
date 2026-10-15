#include "CodeBlock.hpp"

using namespace proxy::prelude;

CodeBlock* CodeBlock::create(const CCSize& size) {
    CodeBlock* instance = new CodeBlock();

    if (instance && instance->init(size)) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

CodeBlock::CodeBlock() : m_info(nullptr) { }

bool CodeBlock::init(const CCSize& size) {
    const Theme::Theme& theme = Theme::getTheme();
    ESCAPE_WHEN(!KeybindNode::init(), false);
    ESCAPE_WHEN(!Border::init(theme.code.background, size), false);

    this->setPaddingY(1);
    this->setPaddingRight(theme.code.scrollbar.size + 1);

    const float fullPaddingY = this->getPaddingY() * 2;
    const float paddingBottom = this->getPaddingBottom();
    CullingList* list = CullingList::create(size - ccp(this->getPaddingX() * 2, fullPaddingY));
    TracklessScrollbar* scrollbarY = TracklessScrollbar::create(true, {
        theme.code.scrollbar.size,
        this->getContentHeight() - fullPaddingY
    }, list->getView());
    m_scrollbars = { nullptr, scrollbarY };
    m_bar = SideBar::create(this, size.height - fullPaddingY, CodeBlock::VIEWS, CodeBlock::ACTIONS);

    scrollbarY->setAnchorPoint(BOTTOM_LEFT);
    scrollbarY->setPosition({ this->getContentWidth() - this->getPaddingRight(), paddingBottom });
    m_bar->setPosition({ 1, paddingBottom });

    this->setNode(list);
    this->reloadCode();
    this->reloadSideBar();
    this->addChild(scrollbarY);
    this->addChild(m_bar);

    return true;
}

void CodeBlock::reloadCode() {
    m_bar->reloadView();
}

void CodeBlock::reloadSideBar() {
    const Theme::Theme& theme = Theme::getTheme();
    TracklessScrollbar* scrollbarX = std::get<0>(m_scrollbars);

    m_bar->reloadState();

    if (Mod::get()->getSettingValue<bool>("minimize-side-menu")) {
        m_bar->scaleToMin();
        this->setPaddingLeft(m_bar->getMinWidth() + 1);
    } else {
        m_bar->scaleToMax();
        this->setPaddingLeft(m_bar->getMaxWidth() + 1);
    }

    if (scrollbarX) {
        const float paddingLeft = this->getPaddingLeft();

        scrollbarX->setPositionX(paddingLeft);
        scrollbarX->setContentWidth(this->getContentWidth() - paddingLeft - this->getPaddingRight());
    }

    this->resizeList();
}

void CodeBlock::updateInfo(std::shared_ptr<HttpInfo> info) {
    if (info != m_info && (info == nullptr || m_info == nullptr || info->getID() != m_info->getID())) {
        m_info = info;

        this->reloadCode();
    }
}

void CodeBlock::setCode(HttpInfo::Content code) {
    m_code = std::move(code.contents);

    this->setupSections(code.type);
    this->setupScrollbars();
    this->setBackgroundColor(Theme::getTheme().code.background);
    this->resizeList();
}

void CodeBlock::setupSections(proxy::enums::ContentType contentType) {
    const Theme::Theme& theme = Theme::getTheme();
    const std::string_view codeView = m_code.view();
    const size_t maxCharacters = Mod::get()->getSettingValue<int64_t>("max-characters-per-line");
    const CCSize fontSize = cocos::getLabelSize("M", theme.code.font.fontName.c_str()) * theme.code.font.fontScale;
    const size_t lineCount = std::count(codeView.begin(), codeView.end(), '\n') + 1;
    const float lineNumberWidth = fontSize.width * std::to_string(lineCount).size() + theme.code.paddingLeft;
    const float lineHeight = fontSize.height + theme.code.font.lineHeight;
    std::vector<JSONTokenizer2::TokenOffset> tokens = contentType == proxy::enums::ContentType::JSON ?
        JSONTokenizer2(codeView).getTokens() :
        std::vector<JSONTokenizer2::TokenOffset>{};
    std::vector<JSONTokenizer2::TokenOffset> sectionTokens;
    std::vector<std::string_view> section;
    std::vector<CullingCell*> cells;
    size_t tokenIndex = 0;

    section.reserve(CodeSectionCell::LINES_PER_SECTION);
    cells.reserve((lineCount + CodeSectionCell::LINES_PER_SECTION - 1) / CodeSectionCell::LINES_PER_SECTION);

    for (size_t off = codeView.find('\n'), lastOff = 0, i = 0, maxLine = 0, total = 1; total <= lineCount; off = codeView.find('\n', off + 1), i++, total++) {
        const bool isEnd = off == std::string_view::npos;
        const std::string_view line = codeView.substr(lastOff, isEnd ? off : off - lastOff);
        maxLine = std::max(maxLine, line.size());
        lastOff = off + 1;

        section.emplace_back(line);

        while (tokenIndex < tokens.size() && tokens[tokenIndex].trueOffset < off) {
            sectionTokens.emplace_back(std::move(tokens[tokenIndex++]));
        }

        if (i + 1 == CodeSectionCell::LINES_PER_SECTION || isEnd) {
            cells.emplace_back(CodeSectionCell::create(
                std::move(section),
                std::move(sectionTokens),
                total - i,
                lineNumberWidth,
                lineNumberWidth + theme.code.paddingCenter + fontSize.width * std::min<size_t>(maxCharacters, maxLine) + theme.code.paddingRight
            ));

            i = -1;
            maxLine = 0;
        }
    }

    reinterpret_cast<CullingList*>(this->getNode())->setCells(cells);
}

void CodeBlock::setupScrollbars() {
    const Theme::Theme& theme = Theme::getTheme();
    TracklessScrollbar* scrollbarX = std::get<0>(m_scrollbars);
    TracklessScrollbar* scrollbarY = std::get<1>(m_scrollbars);
    CullingList* list = reinterpret_cast<CullingList*>(this->getNode());

    if (list->isHorizontalLocked()) {
        if (scrollbarX) {
            m_scrollbars = { nullptr, scrollbarY };

            this->setPaddingBottom(1);
            m_corner->removeFromParent();
            scrollbarX->removeFromParent();
            scrollbarY->setPositionY(this->getPaddingBottom());
            scrollbarY->setContentHeight(this->getContentHeight() - theme.code.scrollbar.size + this->getPaddingY() * 2);
        }
    } else if (!scrollbarX) {
        const float contentWidth = this->getContentWidth();
        const float paddingLeft = this->getPaddingLeft();
        scrollbarX = TracklessScrollbar::create(false, {
            contentWidth - paddingLeft - this->getPaddingRight(),
            theme.code.scrollbar.size
        }, list->getView());
        m_scrollbars = { scrollbarX, scrollbarY };
        m_corner = CCLayerColor::create(theme.code.scrollbar.border);

        this->setPaddingBottom(theme.code.scrollbar.size + 1);
        m_corner->setContentSize({ theme.code.scrollbar.size, theme.code.scrollbar.size });
        m_corner->setAnchorPoint(BOTTOM_RIGHT);
        m_corner->ignoreAnchorPointForPosition(false);
        m_corner->setPosition({ contentWidth - 1, 1 });
        scrollbarX->setPosition({ paddingLeft, 1 });
        scrollbarX->setAnchorPoint(BOTTOM_LEFT);
        scrollbarY->setPositionY(this->getPaddingBottom());
        scrollbarY->setContentHeight(this->getContentHeight() - this->getPaddingY() * 2);
        this->addChild(scrollbarX);
        this->addChild(m_corner);
        list->moveToTop();
    }
}

std::shared_ptr<HttpInfo> CodeBlock::getActiveInfo() const {
    return m_info;
}

void CodeBlock::showMessage(std::string message, const ccColor3B& color) {
    CCNode* node = this->getNode();
    TextAlertPopup* alert = TextAlertPopup::create(std::move(message), 0.5f, 0.6f, 150, "");

    alert->setPosition(node->getContentSize() / 2);
    alert->m_label->setColor(color);
    node->addChild(alert, 100);
}

void CodeBlock::scroll(const float x, const float y) {
    const TableView* tableView = reinterpret_cast<CullingList*>(this->getNode())->getView();
    const float min = 0;
    const CCSize max = -(tableView->m_contentLayer->getContentSize() - tableView->getContentSize());
    const CCPoint position = tableView->m_contentLayer->getPosition();

    tableView->m_contentLayer->setPosition({
        std::clamp(position.x + x, min, max.width),
        std::clamp(position.y + y, min, max.height)
    });
}

void CodeBlock::resizeList() {
    this->getNode()->setContentSize(this->getNode()->getContentSize());
}