#pragma once // Prevents the file from being included multiple times

#include <string>
#include <vector>

// The different kinds of tokens our language recognizes.
enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    SEMICOLON,

    // Literals
    IDENTIFIER,
    STRING_LITERAL,
    // We'll add NUMBER_LITERAL later

    // Special
    END_OF_FILE,
    UNKNOWN // For errors
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