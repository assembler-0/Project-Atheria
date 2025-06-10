#include "lexer.hpp"
#include <cctype> // for isalpha, isalnum

Lexer::Lexer(const std::string& source) : m_source(source) {}

// A very important note on your "Rust-like types" comment:
// The lexer's job is to identify `int32_t` as an IDENTIFIER. That's it.
// It is the *parser's* job later on to understand that this particular
// identifier is being used as a type annotation. This design is flexible
// and correct! It means if you later want `i32` or `MyCoolType`, the
// lexer doesn't need to change at all.

Token Lexer::getNextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return {TokenType::END_OF_FILE, ""};
    }

    char c = advance();

    // Handle identifiers and keywords
    if (isalpha(c) || c == '_') {
        // We've backtracked one character because advance() consumed it.
        m_current_pos--; 
        return makeIdentifier();
    }

    // Handle string literals
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
    return {TokenType::IDENTIFIER, text};
}

Token Lexer::makeString() {
    size_t start = m_current_pos;
    while (peek() != '"' && !isAtEnd()) {
        advance();
    }
    
    // We don't handle unterminated strings for now, but in a real
    // compiler, you would want to report an error here.
    if (isAtEnd()) {
        return {TokenType::UNKNOWN, "Unterminated string."};
    }

    // Consume the closing quote.
    advance();

    // The value of the string is the part *inside* the quotes.
    std::string value = m_source.substr(start, m_current_pos - start - 1);
    return {TokenType::STRING_LITERAL, value};
}