#include "CodeLineCell.hpp"

using namespace proxy::prelude;

CodeLineCell* CodeLineCell::create(const CCSize& size, const size_t lineNumber, const float lineNumberWidth, const HttpInfo::Content& code, JSONTokenizer& tokenizer) {
    CodeLineCell* instance = new CodeLineCell(size, lineNumber, lineNumberWidth, code, tokenizer);

    if (instance && instance->init()) {
        return instance;
    } else {
        delete instance;
        return nullptr;
    }
}

float CodeLineCell::getCodeLineWidth() const {
    return m_lineNumberWidth + Theme::getTheme().code.paddingCenter;
}

CodeLineCell::CodeLineCell(
    const CCSize& size,
    const size_t lineNumber,
    const float lineNumberWidth,
    const HttpInfo::Content& code,
    JSONTokenizer& tokenizer
) : CullingCell(size),
m_code(code),
m_lineNumber(lineNumber),
m_lineNumberWidth(lineNumberWidth) {
    if (code.type == ContentType::BINARY) return;

    for (size_t index = m_code.contents.find('\t'); index != std::string::npos; index = m_code.contents.find('\t')) {
        m_code.contents.replace(index, 1, "  ");
    }

    if (code.type == ContentType::JSON) {
        m_tokens = tokenizer.parseLine(code.contents);
    }
}

void CodeLineCell::initRender() {
    const Theme::Theme& theme = Theme::getTheme();
    const float codeLineWidth = this->getCodeLineWidth();
    const size_t labelLength = std::min<size_t>(m_code.contents.size(), Mod::get()->getSettingValue<int64_t>("max-characters-per-line"));
    MonospaceLabel* lineNumberLabel = MonospaceLabel::create(std::to_string(m_lineNumber), theme.code.font.fontName, theme.code.font.fontScale);
    MonospaceLabel* codeLabel = MonospaceLabel::create(m_code.contents.substr(0, labelLength), theme.code.font.fontName, theme.code.font.fontScale);
    CCLayerColor* numberBackground = CCLayerColor::create(theme.code.background, codeLineWidth, this->getContentHeight());

    m_tokens.orExecute([&theme, codeLabel]() { theme.code.syntax.text.applyTo(codeLabel); })
        .filter([labelLength](const JSONTokenizer::TokenOffset& tokenOffset) { return tokenOffset.offset < labelLength; })
        .forEach([&theme, labelLength, codeLabel](const JSONTokenizer::TokenOffset& tokenOffset) {
            IntStream<size_t>::range(tokenOffset.offset, std::min(labelLength, tokenOffset.offset + tokenOffset.length))
                .map([codeLabel](const int i) { return codeLabel->getChildByIndex<CCSprite>(i); })
                .forEach([&theme, tokenOffset](CCSprite* character) {
                    switch (tokenOffset.token) {
                        case JSONTokenizer::Token::CORRUPT: theme.code.syntax.error.applyTo(character); break;
                        case JSONTokenizer::Token::KEY: theme.code.syntax.key.applyTo(character); break;
                        case JSONTokenizer::Token::STRING: theme.code.syntax.string.applyTo(character); break;
                        case JSONTokenizer::Token::NUMBER: theme.code.syntax.number.applyTo(character); break;
                        case JSONTokenizer::Token::CONSTANT: theme.code.syntax.constant.applyTo(character); break;
                        case JSONTokenizer::Token::TERMINATOR: theme.code.syntax.terminator.applyTo(character); break;
                        case JSONTokenizer::Token::SEPARATOR: theme.code.syntax.separator.applyTo(character); break;
                        case JSONTokenizer::Token::BRACKET: theme.code.syntax.bracket.applyTo(character); break;
                        case JSONTokenizer::Token::KEY_QUOTE: theme.code.syntax.keyQuote.applyTo(character); break;
                        case JSONTokenizer::Token::STRING_QUOTE: theme.code.syntax.stringQuote.applyTo(character); break;
                        default: theme.code.syntax.text.applyTo(character); break;
                    }
                });
        });

    lineNumberLabel->setAnchorPoint(CENTER_RIGHT);
    theme.code.foreground.applyTo(lineNumberLabel);
    lineNumberLabel->setPosition({ m_lineNumberWidth, this->getContentHeight() / 2 });
    codeLabel->setID("code");
    codeLabel->setAnchorPoint(CENTER_LEFT);
    codeLabel->setPosition({ codeLineWidth, this->getContentHeight() / 2 });
    numberBackground->setID("horizontal-align");
    numberBackground->setAnchorPoint(BOTTOM_LEFT);
    numberBackground->setPosition(ZERO_POINT);
    numberBackground->setZOrder(1);
    numberBackground->addChild(lineNumberLabel);
    m_mainLayer->addChild(numberBackground);
    m_mainLayer->addChild(codeLabel);
}