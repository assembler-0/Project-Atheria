#ifndef YL_LEXER_HPP
#define YL_LEXER_HPP

#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>

enum class TokenType {
    Identifier,
    Number,
    String,
    Keyword,
    Symbol,
    Eof,
    Unknown
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

class Lexer {
public:
    Lexer(const std::string& src);
    Token next();

private:
    char peek() const;
    char advance();
    void skipWhitespace();
    Token identifier();
    Token number();
    Token string();

    const std::string& source;
    size_t pos = 0;
    int line = 1;
    int col = 1;
};

#endif // YL_LEXER_HPP
