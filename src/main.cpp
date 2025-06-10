#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"

// MODIFIED: run() now takes the output filename as an argument
void run(const std::string& source, const std::string& out_filename) {
    // 1. Lexer
    Lexer lexer(source);
    std::vector<Token> tokens;
    Token token;
    do {
        token = lexer.getNextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END_OF_FILE);

    // 2. Parser
    Parser parser(tokens);
    std::unique_ptr<ProgramNode> ast = parser.parse();
    if (!ast) {
        std::cerr << "Compilation failed due to parsing errors." << std::endl;
        return;
    }

    // 3. Code Generation
    CodeGen generator;
    generator.generate(ast.get());

    // Optional: You can still dump the IR for debugging!
    std::cout << "--- LLVM IR Generation ---" << std::endl;
    generator.dump();

    // 4. NEW: Emit the actual object file!
    std::cout << "\n--- Emitting Object File ---" << std::endl;
    generator.emitObjectFile(out_filename);
}

// MODIFIED: main() now expects two arguments
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: ac <inputfile> <outputfile.o>" << std::endl;
        return 1;
    }

    std::string in_filename = argv[1];
    std::string out_filename = argv[2];

    std::ifstream file(in_filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << in_filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    run(source, out_filename);

    return 0;
}