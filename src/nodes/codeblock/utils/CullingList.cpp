#include "CullingList.hpp"

CullingList* CullingList::create(const CCSize& size, std::vector<CullingCell*> cells) {
    CullingList* instance = new CullingList();

    if (instance && instance->init(size, std::move(cells))) {
        instance->autorelease();

        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

CullingList::CullingList() : m_view(nullptr), m_disableCulling(false) { }

CullingList::~CullingList() {
    for (CullingCell* cell : m_cells) {
        cell->autorelease();
        cell->removeFromParent();
        cell->release();
    }
}

bool CullingList::init(const CCSize& size, std::vector<CullingCell*> cells) {
    ESCAPE_WHEN(!HoverNode::init(), false);
    ESCAPE_WHEN(!CCLayerColor::initWithColor(ccc4(0x00, 0x00, 0x00, 0x00), size.width, size.height), false);

    m_view = TableView::create(this, this, nullptr, CCRect(0, 0, size.width, size.height));
    m_view->m_peekLimitTop = m_view->m_peekLimitBottom = 0;

    m_view->setMouseEnabled(false);
    this->setMouseEnabled(false);
    this->scheduleUpdate();
    this->addChild(m_view);
    this->setCells(std::move(cells));

    return true;
}

void CullingList::reloadData() {
    const size_t totalCells = m_cells.size();
    float totalHeight;

    if (totalCells) {
        m_cellHeight = m_cells.front()->m_height;
        totalHeight = m_cellHeight * (totalCells - 1) + m_cells.back()->m_mainLayer->getContentHeight();
    } else {
        m_cellHeight = totalHeight = 0;
    }

    m_maxCellWidth = m_scrollLockSteps = 0;
    m_lastOffset = ccp(0, 0);

    m_view->reloadData();

    for (CullingCell* cell : m_cells) {
        m_maxCellWidth = std::max(m_maxCellWidth, cell->m_width);
    }

    m_view->m_contentLayer->setPositionX(0);
    m_view->m_contentLayer->setContentSize({ m_maxCellWidth, totalHeight });
    m_view->m_disableHorizontal = m_maxCellWidth <= m_view->getContentWidth();
    m_view->m_disableVertical = this->shouldDisableVertical();
    // For some reason if m_view->m_disableVertical == false, m_view->m_disableHorizontal is ignored and rather relies on m_view->m_disableMovement
    m_view->m_disableMovement = m_view->m_disableHorizontal && m_view->m_disableVertical;

    if (m_view->m_disableVertical) {
        m_view->m_contentLayer->setContentHeight(m_view->getContentHeight());
    }
}

void CullingList::moveToTop() {
    m_view->m_contentLayer->setPositionY(m_view->getContentHeight() - m_view->m_contentLayer->getContentHeight());
}

void CullingList::moveToBottom() {
    m_view->m_contentLayer->setPositionY(0);
}

void CullingList::toggleCulling(const bool enabled) {
    m_disableCulling = !enabled;
}

void CullingList::setCells(std::vector<CullingCell*> cells) {
    for (CullingCell* cell : m_cells) {
        cell->autorelease();
        cell->removeFromParent();
        cell->release();
    }

    m_cells = std::move(cells);

    m_activeCells.clear();
    this->reloadData();
    this->moveToTop();

    if (m_disableCulling) {
        this->update(0);
    }
}

void CullingList::setContentWidth(const float width) {
    CCLayerColor::setContentWidth(width);

    if (m_view != nullptr) {
        m_view->setContentHeight(width);
        this->reloadData();
    }
}

void CullingList::setContentHeight(const float height) {
    CCLayerColor::setContentHeight(height);

    if (m_view != nullptr) {
        m_view->setContentHeight(height);
        this->reloadData();
    }
}

void CullingList::setContentSize(const CCSize& size) {
    CCLayerColor::setContentSize(size);

    if (m_view != nullptr) {
        m_view->setContentSize(size);
        this->reloadData();
    }
}

void CullingList::scrollWheel(const float y, const float x) {
    m_view->m_contentLayer->setPositionX(std::clamp<float>(m_view->m_contentLayer->getPositionX() + x, -(
        std::max<float>(m_view->m_contentLayer->getContentWidth() - m_view->getContentWidth(), 0)
    ), 0));
    m_view->scrollWheel(y, x);
}

bool CullingList::isHorizontalLocked() {
    return m_view->m_disableHorizontal;
}

bool CullingList::shouldDisableVertical() {
    return m_view->m_contentLayer->getContentHeight() <= m_view->getContentHeight();
}

CullingCell* CullingList::renderCell(const size_t index) {
    CullingCell* cell = m_cells.at(index);

    if (!cell->isActive()) {
        m_view->m_contentLayer->addChild(cell);
        cell->render();
        cell->setPositionY(m_view->m_contentLayer->getContentHeight() - (index + 1) * m_cellHeight);

        cell->m_indexPath.m_row = index;
        cell->m_tableView = m_view;
    }

    return cell;
}

void CullingList::horizontalRender(CullingCell* cell) {
    CCNode* node = cell->getChildByIDRecursive("horizontal-align");

    if (node != nullptr) {
        node->setPositionX(-m_view->m_contentLayer->getPositionX() + node->getScaledContentWidth() * node->getAnchorPoint().x);
    }
}

void CullingList::update(const float dt) {
    HoverNode::update(dt);

    const size_t size = m_cells.size();
    const CCPoint contentOffset = m_view->m_contentLayer->getPosition();
    const bool moved = contentOffset != m_lastOffset;
    const bool renderAll = m_view->m_disableVertical || m_disableCulling;
    const bool shouldSkipCulling = renderAll && m_activeCells.size() == size;
    const CCSize& viewSize = m_view->getContentSize();

    // Alignment snapping
    if (m_scrollLockSteps > 0) {
        m_scrollLockSteps--;
    }

    if (moved) {
        if (m_scrollLockSteps == 0) {
            m_xAxisLocked = std::abs(contentOffset.x - m_lastOffset.x) > std::abs(contentOffset.y - m_lastOffset.y);
            m_scrollLockSteps = Mod::get()->getSettingValue<int64_t>("snapping-aggression");
        }

        if (m_xAxisLocked) {
            m_lastOffset.x = contentOffset.x;

            m_view->m_contentLayer->setPositionY(m_lastOffset.y);
        } else {
            m_lastOffset.y = contentOffset.y;

            m_view->m_contentLayer->setPositionX(m_lastOffset.x);
        }
    }

    // Why does this need to happen every step and not in moved
    m_view->m_contentLayer->setPositionX(std::clamp(m_lastOffset.x, std::min(viewSize.width - m_maxCellWidth, 0.0f), 0.0f));

    for (CullingCell* cell : m_activeCells) {
        this->horizontalRender(cell);
    }

    ESCAPE_WHEN(m_activeCells.size() && (!moved || shouldSkipCulling),);

    // The actual culling part
    size_t offset = 0;
    size_t end = size;

    if (!renderAll) {
        offset = std::clamp<size_t>((m_view->m_contentLayer->getContentHeight() - viewSize.height + m_lastOffset.y) / m_cellHeight, 0, size - 1);
        // TODO: If I don't feel lazy, integrate a offset modulo into this calculation to figure out if the last cell is already out of bounds as this calculation
        // just checks how many cells can fit, not how many cells are needed to fill the screen at the current scroll
        end = std::min<size_t>(offset + std::ceil(viewSize.height / m_cellHeight) + 1, size);
    }

    Stream<CullingCell*> newActiveCells;

    for (size_t i = offset; i < end; i++) {
        CullingCell* cell = this->renderCell(i);

        this->horizontalRender(cell);
        newActiveCells.push_back(cell);
    }

    for (CullingCell* cell : m_activeCells) {
        if (newActiveCells.includes(cell)) continue;

        cell->removeFromParentAndCleanup(false);
        cell->discard();
    }

    m_activeCells = std::move(newActiveCells);
}

int CullingList::numberOfRowsInSection(unsigned int section, TableView* view) {
    return 0;
}

unsigned int CullingList::numberOfSectionsInTableView(TableView* view) {
    return 0;
}

float CullingList::cellHeightForRowAtIndexPath(CCIndexPath& indexPath, TableView* view) {
    return m_cellHeight;
}

TableViewCell* CullingList::cellForRowAtIndexPath(CCIndexPath& indexPath, TableView* view) {
    return nullptr;
}

void CullingList::onHover() {
    this->setMouseEnabled(true);
}

void CullingList::unHover() {
    this->setMouseEnabled(false);
}