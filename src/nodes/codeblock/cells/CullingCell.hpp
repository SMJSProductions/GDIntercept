#pragma once

#include "../../../include.hpp"

class CullingCell : public TableViewCell {
public:
    CullingCell(const CCSize& size);
    CullingCell(const ZStringView id, const CCSize& size);
    ~CullingCell();
    bool isActive();
    void render();
    virtual void discard();
protected:
    // No Nodes should be cached through this method, that defeats the purpose of culling
    virtual void firstRender() { };
    virtual void initRender() = 0;
private:
    bool m_rendered;
    bool m_active;
};