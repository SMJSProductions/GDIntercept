#include "CullingCell.hpp"

CullingCell::CullingCell(const CCSize& size) : CullingCell("", size) { }

CullingCell::CullingCell(const ZStringView id, const CCSize& size) : TableViewCell(id.c_str(), size.width, size.height),
m_active(false),
m_rendered(false) {
    m_listType = BoomListType::Default;

    this->retain();
    // Apparently you giving TableViewCell a size is useless because it just randomly picks a size
    m_mainLayer->setContentSize(size);
    m_mainLayer->setAnchorPoint(BOTTOM_LEFT);
}

CullingCell::~CullingCell() {
    m_mainLayer->removeAllChildren();
}

bool CullingCell::isActive() {
    return m_active;
}

void CullingCell::render() {
    m_active = true;

    if (!m_rendered) {
        m_rendered = true;

        this->firstRender();
    }

    this->setContentSize({ m_width, m_height });
    this->initRender();
    // In some weird edge cases, the cell may automatically be set to invisible. no clue why
    this->setVisible(true);
}

void CullingCell::discard() {
    m_active = false;

    m_mainLayer->removeAllChildren();
}