#include "JSONTokenizer2.hpp"

JSONTokenizer2::JSONTokenizer2(const std::string_view code): m_code(code), m_off(0), m_skipped(0), m_errorState(false) {
    if (m_code.empty()) return;

    m_tokens = this->parseValue();

    if (m_off) {
        m_tokens.emplace_back(this->setupSyntaxError(m_off, m_code.size()));
    }
}

const std::vector<JSONTokenizer2::TokenOffset>& JSONTokenizer2::getTokens() const& {
    return std::move(m_tokens);
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::getTokens() && {
    return std::move(m_tokens);
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseValue() {
    constexpr std::string NUMBERS = "-0123456789";

    if (m_code[m_off] == '[') {
        return this->parseArray();
    } else if (m_code[m_off] == '{') {
        return this->parseObject();
    } else if (m_code[m_off] == '"' || m_code[m_off] == '\'') {
        return this->parseString(false);
    } else if (NUMBERS.find(m_code[m_off]) != std::string_view::npos) {
        return this->parseNumber();
    } else if (std::optional<TokenOffset> constant = this->parseConstant()) {
        return { std::move(constant).value() };
    } else {
        return {};
    }
}

std::optional<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseConstant() {
    const std::string_view cutCode = m_code.substr(m_off);

    if (cutCode.starts_with("true")) {
        return this->setupLengthToken(Token::BOOLEAN_CONST, 4);
    } else if (cutCode.starts_with("false")) {
        return this->setupLengthToken(Token::BOOLEAN_CONST, 5);
    } else if (cutCode.starts_with("null")) {
        return this->setupLengthToken(Token::NULL_CONST, 4);
    } else {
        return std::nullopt;
    }
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseNumber() {
    constexpr const char* NUMBERS = "0123456789";
    const size_t start = m_off;
    std::vector<TokenOffset> tokens;

    for (size_t i = 0; i < 2; i++) {
        // Handle sign
        if (m_off >= m_code.size()) {
            break;
        } else if (m_off < m_code.size() && m_code[m_off] == '-') {
            tokens.emplace_back(this->setupLengthToken(Token::NUMBER_NEGATIVE, 1));
            tokens.emplace_back(this->setupSubstrToken(Token::NUMBER, m_code.find_first_not_of(NUMBERS, m_off)));
        } else {
            tokens.emplace_back(this->setupSubstrToken(Token::NUMBER, m_code.find_first_not_of(NUMBERS, m_off)));
        }

        // Handle decimal digits
        if (m_errorState) {
            break;
        } else if (m_off < m_code.size() && m_code[m_off] == '.') {
            tokens.emplace_back(this->setupLengthToken(Token::NUMBER_DECIMAL, 1));
            tokens.emplace_back(this->setupSubstrToken(Token::NUMBER, m_code.find_first_not_of(NUMBERS, m_off)));

            if (m_errorState) {
                break;
            }
        }

        // Handle e-notation
        if (i == 0 && m_off < m_code.size() && (m_code[m_off] == 'e' || m_code[m_off] == 'E')) {
            tokens.emplace_back(this->setupLengthToken(Token::NUMBER_E_NOTATION, 1));
        } else {
            // No e-notation so no reason to parse a secondary number
            return tokens;
        }
    }

    // Early end was encountered so break and provide an error
    return { this->setupSyntaxError(start, m_off) };
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseString(const bool isKey) {
    const Token textToken = isKey ? Token::KEY : Token::STRING;
    const Token quoteToken = isKey ? Token::KEY_QUOTE : Token::STRING_QUOTE;
    const Token escapeToken = isKey ? Token::KEY_ESCAPE : Token::STRING_ESCAPE;
    const char quote = m_code[m_off];
    const size_t start = m_off;
    const std::string search = std::string(1, quote) + "\\\n";
    std::vector<TokenOffset> tokens{ this->setupLengthToken(quoteToken, 1) };
    size_t next = m_code.find_first_of(search, m_off);

    // Keep iterating as long as the string is still on the same line
    while (next != std::string_view::npos && m_code[next] != '\n') {
        if (m_off != next) {
            tokens.emplace_back(this->setupSubstrToken(textToken, next, this->countWhitespace(m_off, next)));
        }

        if (m_code[next] == quote) {
            tokens.emplace_back(this->setupLengthToken(quoteToken, 1));

            return tokens;
        } else if (m_code[next + 1] == '\n') {
            // Prevent that a newline gets treated as escapable
            next++;

            break;
        } else {
            tokens.emplace_back(this->setupLengthToken(escapeToken, 2));
        }

        next = m_code.find_first_of(search, m_off);
    }

    return { this->setupSyntaxError(start, next) };
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseArray() {
    std::vector<TokenOffset> tokens;

    tokens.emplace_back(this->setupLengthToken(Token::BRACKET, 1));
    this->skipWhiteSpace();

    while (m_off < m_code.size()) {
        if (this->parseObjectEnd(']', tokens)) {
            return tokens;
        } else {
            this->parseObjectValue(']', tokens);
        }

        // If it reached an unexpected end, break. Otherwise check if it's expecting another value or at the end of the array
        if (m_off >= m_code.size()) {
            break;
        } else if (m_code[m_off] == ',') {
            tokens.emplace_back(this->setupLengthToken(Token::TERMINATOR, 1));
            this->skipWhiteSpace();
        }
    }

    // Treat the last token as an unexpected end
    tokens.back().token = Token::SYNTAX_ERROR;

    return tokens;
}

std::vector<JSONTokenizer2::TokenOffset> JSONTokenizer2::parseObject() {
    std::vector<TokenOffset> tokens;

    tokens.emplace_back(this->setupLengthToken(Token::CURLY_BRACKET, 1));
    this->skipWhiteSpace();

    while (m_off < m_code.size()) {
        if (m_code[m_off] == '"' || m_code[m_off] == '\'') {
            std::vector<TokenOffset> keyTokens = this->parseString(true);

            tokens.insert(tokens.end(), std::make_move_iterator(keyTokens.begin()), std::make_move_iterator(keyTokens.end()));
            this->skipWhiteSpace();
        } else if (this->parseObjectEnd('}', tokens)) {
            return tokens;
        } else  {
            m_errorState = true;
        }

        // If encountering more content or a sudden end of the object. Treat it as a syntax error
        if (const size_t next = m_code.find_first_of(":,}", m_off); next != m_off) {
            tokens.emplace_back(this->setupSyntaxError(m_off, next));
        }

        if (m_off < m_code.size() && m_code[m_off] == ':') {
            tokens.emplace_back(this->setupLengthToken(Token::SEPARATOR, 1));
            this->skipWhiteSpace();
            this->parseObjectValue('}', tokens);
        }

        // If it reached an unexpected end, break. Otherwise check if it's expecting another value or at the end of the object
        if (m_off >= m_code.size()) {
            break;
        } else if (m_code[m_off] == ',') {
            tokens.emplace_back(this->setupLengthToken(Token::TERMINATOR, 1));
            this->skipWhiteSpace();
        }
    }

    // Treat the last token as an unexpected end
    tokens.back().token = Token::SYNTAX_ERROR;

    return tokens;
}

bool JSONTokenizer2::parseObjectEnd(const char objectEndDelimiter, std::vector<TokenOffset>& tokens) {
    if (m_code[m_off] == objectEndDelimiter) {
        TokenOffset& last = tokens.back();

        if (last.token == Token::TERMINATOR) {
            last.token = Token::SYNTAX_ERROR;
        }

        tokens.emplace_back(this->setupLengthToken(objectEndDelimiter == ']' ? Token::BRACKET : Token::CURLY_BRACKET, 1));

        return true;
    } else {
        return false;
    }
}

void JSONTokenizer2::parseObjectValue(const char objectEndDelimiter, std::vector<TokenOffset>& tokens) {
    std::vector<TokenOffset> valueTokens = this->parseValue();
    const size_t noValue = valueTokens.empty();

    if (!noValue) {
        tokens.insert(tokens.end(), std::make_move_iterator(valueTokens.begin()), std::make_move_iterator(valueTokens.end()));
        this->skipWhiteSpace();
    }

    // Check if something besides the initially found value exists in the value section, consider this a syntax error
    // Otherwise consider the previous token a syntax error if no value was present either
    if (const size_t next = m_code.find_first_of(std::string(1, objectEndDelimiter) + ',', m_off); next != m_off) {
        tokens.emplace_back(this->setupSyntaxError(m_off, next));
    } else if (noValue) {
        m_errorState = true;
    }
}