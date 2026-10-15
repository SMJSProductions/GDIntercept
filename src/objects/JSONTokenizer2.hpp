#pragma once

#include "../include.hpp"

class JSONTokenizer2 {
public:
    enum class Token {
        SYNTAX_ERROR,
        KEY,
        KEY_ESCAPE,
        STRING,
        STRING_ESCAPE,
        NUMBER,
        NUMBER_NEGATIVE,
        NUMBER_DECIMAL,
        NUMBER_E_NOTATION,
        NULL_CONST,
        BOOLEAN_CONST,
        TERMINATOR,
        SEPARATOR,
        CURLY_BRACKET,
        BRACKET,
        KEY_QUOTE,
        STRING_QUOTE
    };

    struct TokenOffset {
        Token token;
        size_t offset;
        size_t length;
        size_t trueOffset;
    };

    const std::string_view m_code;
    std::vector<TokenOffset> m_tokens;
    size_t m_off;
    size_t m_skipped;
    bool m_errorState;

    JSONTokenizer2(const std::string_view code);
    const std::vector<TokenOffset>& getTokens() const&;
    std::vector<TokenOffset> getTokens() &&;
private:
    std::vector<TokenOffset> parseValue();
    std::optional<TokenOffset> parseConstant();
    std::vector<TokenOffset> parseNumber();
    std::vector<TokenOffset> parseString(const bool isKey);
    std::vector<TokenOffset> parseArray();
    std::vector<TokenOffset> parseObject();
    bool parseObjectEnd(const char objectEndDelimiter, std::vector<TokenOffset>& tokens);
    void parseObjectValue(const char objectEndDelimiter, std::vector<TokenOffset>& tokens);

    inline void skipWhiteSpace() {
        const size_t oldOff = m_off;

        m_off = m_code.find_first_not_of(" \t\r\n", m_off);
        m_skipped += m_off - oldOff;
    }

    inline TokenOffset setupLengthToken(const Token token, const size_t length, const size_t skipped = 0) {
        const TokenOffset offset = {
            m_errorState ? Token::SYNTAX_ERROR : token,
            m_off - m_skipped,
            length - skipped,
            m_off
        };

        m_errorState = false;
        m_skipped += skipped;
        m_off += length;

        if (!length) {
            m_errorState = true;
        }

        return offset;
    }

    inline TokenOffset setupSubstrToken(const Token token, const size_t nextTokenStart, const size_t skipped = 0) {
        return this->setupLengthToken(token, std::min(nextTokenStart, m_code.size()) - m_off, skipped);
    }

    inline TokenOffset setupSyntaxError(const size_t start, const size_t nextTokenStart) {
        m_off = start;

        return this->setupSubstrToken(Token::SYNTAX_ERROR, nextTokenStart, this->countWhitespace(start, nextTokenStart));
    }

    inline size_t countWhitespace(const size_t from, const size_t until) {
        size_t skipped = 0;

        for (size_t i = from; i < until; i++) {
            const char character = m_code[i];

            if (character == ' ' || character == '\t' || character == '\r' || character == '\n') {
                skipped++;
            }
        }

        return skipped;
    }
};