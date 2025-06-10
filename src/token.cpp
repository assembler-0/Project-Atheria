#include "token.hpp"
#include <iostream>

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:     return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:    return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:     return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:    return "RIGHT_BRACE";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::END_OF_FILE:    return "END_OF_FILE";
        case TokenType::PLUS:    return "PLUS";
        case TokenType::MINUS:    return "MINUS";
        case TokenType::STAR:    return "STAR";
        case TokenType::SLASH:    return "SLASH";
        case TokenType::EQUAL:    return "EQUAL";
        case TokenType::NUMBER_LITERAL:    return "NUMBER_LITERAL";
        case TokenType::RETURN:    return "RETURN";
        case TokenType::COMMA:    return "COMMA";
        case TokenType::AUTO:    return "AUTO";
        default:                        return "UNKNOWN";
    }
}

void Token::print() const {
    std::cout << "Token( " << tokenTypeToString(type) 
              << ", \"" << value << "\" )" << std::endl;
}