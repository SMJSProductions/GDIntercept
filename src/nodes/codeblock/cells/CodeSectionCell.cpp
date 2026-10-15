#include "CodeSectionCell.hpp"

CodeSectionCell* CodeSectionCell::create(
    std::vector<std::string_view> code,
    std::vector<JSONTokenizer2::TokenOffset> tokens,
    const size_t lineNumberOffset,
    const float lineNumberWidth,
    const float width
) {
    const size_t codeSize = code.size();
    const Theme::Theme& theme = Theme::getTheme();
    const CCSize fontSize = cocos::getLabelSize("M", theme.code.font.fontName.c_str()) * theme.code.font.fontScale;
    const float lineHeight = fontSize.height + theme.code.font.lineHeight;
    CodeSectionCell* instance = new CodeSectionCell(std::move(code), std::move(tokens), lineNumberOffset, lineNumberWidth, {
        width,
        lineHeight * CodeSectionCell::LINES_PER_SECTION
    }, lineHeight * codeSize);

    if (instance && instance->init()) {
        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

CodeSectionCell::CodeSectionCell(
    std::vector<std::string_view> code,
    std::vector<JSONTokenizer2::TokenOffset> tokens,
    const size_t lineNumberOffset,
    const float lineNumberWidth,
    const CCSize size,
    const float mainLayerHeight
) : CullingCell(size),
m_code(std::move(code)),
m_tokens(std::move(tokens)),
m_lineNumberOffset(lineNumberOffset),
m_lineNumberWidth(lineNumberWidth) {
    m_mainLayer->setAnchorPoint(TOP_LEFT);
    m_mainLayer->setPositionY(size.height);
    m_mainLayer->setContentHeight(mainLayerHeight);
}

float CodeSectionCell::getLineNumberWidth() {
    return m_lineNumberWidth + Theme::getTheme().code.paddingCenter;
}

void CodeSectionCell::initRender() {
    const Theme::Theme& theme = Theme::getTheme();
    const float halfLineHeight = theme.code.font.lineHeight / 2;
    const float scaledLineHeight = theme.code.font.lineHeight / theme.code.font.fontScale;
    StringBuffer lineNumbersBuffer;
    StringBuffer linesBuffer;

    for (size_t i = 0; i < m_code.size(); i++) {
        lineNumbersBuffer.append("{}\n", i + m_lineNumberOffset);
    }

    for (const std::string_view line : m_code) {
        linesBuffer.append("{}\n", line);
    }

    std::string lineNumbers = lineNumbersBuffer.str();
    std::string lines = linesBuffer.str();

    lineNumbers.pop_back();
    lines.pop_back();

    Label* lineNumberLabel = Label::create(lineNumbers, theme.code.font.fontName);
    Label* linesLabel = Label::create(lines, theme.code.font.fontName);
    CCLayerColor* numberBackground = CCLayerColor::create(theme.code.background);

    theme.code.foreground.applyTo(lineNumberLabel);
    theme.code.syntax.text.applyTo(linesLabel);
    lineNumberLabel->setScale(theme.code.font.fontScale);
    lineNumberLabel->setLineSpacing(scaledLineHeight);
    lineNumberLabel->setAlignment(Label::Alignment::Right);
    linesLabel->setScale(theme.code.font.fontScale);
    linesLabel->setLineSpacing(scaledLineHeight);

    numberBackground->setID("horizontal-align");
    numberBackground->addChild(lineNumberLabel);
    numberBackground->setLayout(SimpleRowLayout::create()
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::Fit)
        ->setPadding({
            m_lineNumberWidth - lineNumberLabel->getScaledContentWidth(),
            0,
            theme.code.paddingCenter,
            0
        }));
    m_mainLayer->addChild(numberBackground);
    m_mainLayer->addChild(linesLabel);
    m_mainLayer->setLayout(SimpleRowLayout::create()
        ->setPadding({
            0,
            halfLineHeight,
            theme.code.paddingRight,
            halfLineHeight
        })
        ->setGap(0)
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisAlignment(CrossAxisAlignment::Center));
    numberBackground->setZOrder(1);
    this->renderSyntax(linesLabel);
}

void CodeSectionCell::renderSyntax(Label* label) {
    const Theme::Theme& theme = Theme::getTheme();

    if (m_tokens.empty()) {
        return theme.code.syntax.text.applyTo(label);
    }

    const size_t firstOffset = m_tokens.front().offset;

    for (const JSONTokenizer2::TokenOffset& tokenOffset : m_tokens) {
        const Color* color = &theme.code.syntax.text;

        switch (tokenOffset.token) {
            case JSONTokenizer2::Token::SYNTAX_ERROR: color = &theme.code.syntax.error; break;
            case JSONTokenizer2::Token::KEY: color = &theme.code.syntax.key; break;
            case JSONTokenizer2::Token::KEY_ESCAPE: color = &theme.code.syntax.key; break; // TODO:
            case JSONTokenizer2::Token::STRING: color = &theme.code.syntax.string; break;
            case JSONTokenizer2::Token::STRING_ESCAPE: color = &theme.code.syntax.string; break; // TODO:
            case JSONTokenizer2::Token::NUMBER: color = &theme.code.syntax.number; break;
            case JSONTokenizer2::Token::NUMBER_NEGATIVE: color = &theme.code.syntax.number; break; // TODO:
            case JSONTokenizer2::Token::NUMBER_DECIMAL: color = &theme.code.syntax.number; break; // TODO:
            case JSONTokenizer2::Token::NUMBER_E_NOTATION: color = &theme.code.syntax.number; break; // TODO:
            case JSONTokenizer2::Token::NULL_CONST: color = &theme.code.syntax.constant; break; // TODO:
            case JSONTokenizer2::Token::BOOLEAN_CONST: color = &theme.code.syntax.constant; break; // TODO:
            case JSONTokenizer2::Token::TERMINATOR: color = &theme.code.syntax.terminator; break;
            case JSONTokenizer2::Token::SEPARATOR: color = &theme.code.syntax.separator; break;
            case JSONTokenizer2::Token::CURLY_BRACKET: color = &theme.code.syntax.bracket; break; // TODO:
            case JSONTokenizer2::Token::BRACKET: color = &theme.code.syntax.bracket; break;
            case JSONTokenizer2::Token::KEY_QUOTE: color = &theme.code.syntax.keyQuote; break;
            case JSONTokenizer2::Token::STRING_QUOTE: color = &theme.code.syntax.stringQuote; break;
        }

        for (size_t i = 0; i < tokenOffset.length; i++) {
            label->setCharColor(i + tokenOffset.offset - firstOffset, *color);
        }
    }
}