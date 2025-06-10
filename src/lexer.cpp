#include "lexer.hpp"
#include <cctype> // for isalpha, isalnum, isdigit

Lexer::Lexer(const std::string& source) : m_source(source) {}

// Helper function to check for keywords
static TokenType checkKeyword(const std::string& text) {
    if (text == "return") return TokenType::RETURN;
    if (text == "auto") return TokenType::AUTO;
    return TokenType::IDENTIFIER;
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return {TokenType::END_OF_FILE, ""};
    }

    char c = advance();

    // Handle multi-character tokens first
    if (isalpha(c) || c == '_') {
        m_current_pos--; // Backtrack to include the first character
        return makeIdentifier();
    }

    if (isdigit(c)) {
        m_current_pos--; // Backtrack to include the first character
        return makeNumber();
    }

    if (c == '"') {
        return makeString();
    }

    // Handle single-character tokens
    switch (c) {
        case '(': return {TokenType::LEFT_PAREN, "("};
        case ')': return {TokenType::RIGHT_PAREN, ")"};
        case '{': return {TokenType::LEFT_BRACE, "{"};
        case '}': return {TokenType::RIGHT_BRACE, "}"};
        case ';': return {TokenType::SEMICOLON, ";"};
        case '+': return {TokenType::PLUS, "+"};
        case '-': return {TokenType::MINUS, "-"};
        case '*': return {TokenType::STAR, "*"};
        case '/': return {TokenType::SLASH, "/"};
        case '=': return {TokenType::EQUAL, "="};
        case ',': return {TokenType::COMMA, ","};
    }

    return {TokenType::UNKNOWN, std::string(1, c)};
}

// --- Private Helper Methods ---

bool Lexer::isAtEnd() {
    return m_current_pos >= m_source.length();
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return m_source[m_current_pos];
}

char Lexer::advance() {
    if (!isAtEnd()) m_current_pos++;
    return m_source[m_current_pos - 1];
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance();
                break;
            default:
                return;
        }
    }
}

Token Lexer::makeIdentifier() {
    size_t start = m_current_pos;
    while (isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string text = m_source.substr(start, m_current_pos - start);
    TokenType type = checkKeyword(text);
    return {type, text};
}

Token Lexer::makeString() {
    size_t start = m_current_pos;
    while (peek() != '"' && !isAtEnd()) {
        advance();
    }

    if (isAtEnd()) {
        return {TokenType::UNKNOWN, "Unterminated string."};
    }

    advance(); // Consume the closing quote.
    std::string value = m_source.substr(start, m_current_pos - start - 1);
    return {TokenType::STRING_LITERAL, value};
}

Token Lexer::makeNumber() {
    size_t start = m_current_pos;
    while (isdigit(peek())) {
        advance();
    }
    std::string value = m_source.substr(start, m_current_pos - start);
    return {TokenType::NUMBER_LITERAL, value};
}