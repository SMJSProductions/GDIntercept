#pragma once

#include "../../../include.hpp"
#include "CullingCell.hpp"
#include "../../../proxy/Proxy.hpp"
#include "../../../objects/JSONTokenizer2.hpp"

class CodeSectionCell : public CullingCell {
public:
    static constexpr size_t LINES_PER_SECTION = 50;

    static CodeSectionCell* create(
        std::vector<std::string_view> code,
        std::vector<JSONTokenizer2::TokenOffset> tokens,
        const size_t lineNumberOffset,
        const float lineNumberWidth,
        const float width
    );
private:
    const std::vector<std::string_view> m_code;
    const std::vector<JSONTokenizer2::TokenOffset> m_tokens;
    const size_t m_lineNumberOffset;
    const float m_lineNumberWidth;
public:
    CodeSectionCell(
        std::vector<std::string_view> code,
        std::vector<JSONTokenizer2::TokenOffset> tokens,
        const size_t lineNumberOffset,
        const float lineNumberWidth,
        const CCSize size,
        const float mainLayerHeight
    );

    float getLineNumberWidth();
protected:
    void initRender() override;
private:
    void renderSyntax(Label* label);
};