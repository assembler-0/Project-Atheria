#pragma once // Prevents the file from being included multiple times

#include <string>
#include <vector>

// The different kinds of tokens our language recognizes.
enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    SEMICOLON,
    PLUS,       // +
    MINUS,      // -
    STAR,       // *
    SLASH,      // /
    EQUAL,      // =
    COMMA,      // ,

    // Literals
    IDENTIFIER,
    STRING_LITERAL,
    NUMBER_LITERAL, // e.g., 123

    // Keywords
    RETURN,     // The 'return' keyword

    // Special
    END_OF_FILE,
    UNKNOWN
};

// A simple helper function to convert a TokenType to a string for printing.
// This is incredibly useful for debugging!
std::string tokenTypeToString(TokenType type);

// Our Token struct
struct Token {
    TokenType type;
    std::string value; // The actual text of the token, e.g., "print"

    // A handy method for debugging
    void print() const;
};