#pragma once
#include "token.hpp"

class Lexer {
public:
    // Constructor takes the source code to be tokenized
    Lexer(const std::string& source);

    // The main function of the lexer. Returns the next token.
    Token getNextToken();

private:
    std::string m_source;
    size_t m_current_pos = 0;

    // Helper functions
    char peek(); // Look at the current character without consuming it
    char advance(); // Consume the current character and move to the next
    bool isAtEnd(); // Check if we've consumed all characters
    void skipWhitespace(); // Skips spaces, tabs, newlines

    // Token-specific helpers]
    Token makeNumber();
    Token makeIdentifier();
    Token makeString();
};