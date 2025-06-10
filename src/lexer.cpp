#include "lexer.hpp"
#include <cctype>
#include <unordered_set>

namespace {
    std::unordered_set<std::string> keywords = {
        "if", "else", "while", "return", "func", "let", "true", "false"
    };

    bool isSymbolChar(char c) {
        return std::string("+-*/=(){};,:<>!&|[]").find(c) != std::string::npos;
    }
}

Token Lexer::nextToken() {
    while (pos < source.size() && std::isspace(source[pos])) ++pos;

    if (pos >= source.size()) return { TokenType::EndOfFile, "", pos };

    size_t start = pos;

    if (std::isalpha(source[pos]) || source[pos] == '_') {
        while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) ++pos;
        std::string word = source.substr(start, pos - start);
        if (keywords.count(word)) return { TokenType::Keyword, word, start };
        return { TokenType::Identifier, word, start };
    }

    if (std::isdigit(source[pos])) {
        while (pos < source.size() && std::isdigit(source[pos])) ++pos;
        return { TokenType::Number, source.substr(start, pos - start), start };
    }

    if (source[pos] == '"') {
        ++pos; // skip opening quote
        while (pos < source.size() && source[pos] != '"') {
            if (source[pos] == '\') ++pos;
            ++pos;
        }
        ++pos; // skip closing quote
        return { TokenType::String, source.substr(start + 1, pos - start - 2), start };
    }

    if (isSymbolChar(source[pos])) {
        return { TokenType::Symbol, std::string(1, source[pos++]), start };
    }

    return { TokenType::Invalid, std::string(1, source[pos++]), start };
}
