//
// Created by ayanami on 12/3/24.
//

#include "parser.h"
NumNode* Parser::parseNum() {
    auto token = tokenizer->peek();
    if(token.type != Token::TokenType::NUM) {
        throw std::runtime_error("Invalid number token: " + Token::tk2Str(token.type));
    }
    tokenizer->eat(token.type);
    if (!token.value.has_value()) {
        throw std::runtime_error("number token value should not be null");
    }
    return new NumNode(str2Number(token.value.value()));
}
StringNode* Parser::parseString() {
    auto token = tokenizer->peek();
    if(token.type != Token::TokenType::LITERAL) {
        std::string s = "Invalid string token: " + Token::tk2Str(token.type);
        throw std::runtime_error(s);
    }
    tokenizer->eat(token.type);
    if (!token.value.has_value()) {
        auto s= "string token value should not be null";
        throw std::runtime_error(s);
    }
    return new StringNode(token.value.value());
}
VarNode* Parser::parseVar() {
    auto token = tokenizer->peek();
    if(token.type != Token::TokenType::VAR) {
        throw std::runtime_error("Invalid variable token: " + tk2Str(token.type));
    }
    tokenizer->eat(token.type);
    return new VarNode(token.value.value());
}
// factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN | variable | STRING
ASTNode* Parser::factor() {
    auto token = tokenizer->peek();
    if (token.type == Token::TokenType::OP_ADD || token.type == Token::TokenType::OP_SUB) {
        tokenizer->eat(token.type);
        ASTNode* actual_factor = factor();
        return new UnaryOpNode(actual_factor, token.type);
    }
    if(token.type == Token::TokenType::NUM) {
        return parseNum();
    }
    if(token.type == Token::TokenType::LPAREN) {
        tokenizer->eat(token.type);
        ASTNode* expr_node = expr();
        tokenizer->eat(Token::TokenType::RPAREN);
        return expr_node;
    }
    if(token.type == Token::TokenType::VAR) {
        return parseVar();
    }
    if (token.type == Token::TokenType::LITERAL) {
        return parseString();
    }
    throw std::runtime_error("Invalid factor token: " + Token::tk2Str(token.type));
}

// factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN | variable
// term_p1 : factor (POW term_p1)* // right associative
// term_p2 : term_p1 ((MUL | DIV | MOD) term_p1)*
// term_p3 : term_p2 ((PLUS | MINUS) term_p2)*
// term_p4 : term_p3 ((GT | EQ | LT | GE | LE | NE) term_p3)*
// expr : term_p4
ASTNode* Parser::term_p1() {
    ASTNode* node = factor();
    auto token = tokenizer->peek();
    while(token.type == Token::TokenType::OP_POW) {
        tokenizer->eat(token.type);
        // node = new BinOpNode(node, factor(), token.type);
        node = new BinOpNode(node, term_p1(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::term_p2() {
    ASTNode* node = term_p1();
    auto token = tokenizer->peek();
    static std::set<Token::TokenType> p2_tokens = {
        Token::TokenType::OP_MUL,
        Token::TokenType::OP_DIV,
        Token::TokenType::OP_MOD
    };
    while(Token::typeIn(token.type, p2_tokens)) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, term_p1(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::term_p3() {
    ASTNode* node = term_p2();
    auto token = tokenizer->peek();
    static std::set<Token::TokenType> p3_tokens = {
        Token::TokenType::OP_ADD,
        Token::TokenType::OP_SUB,
    };
    while(Token::typeIn(token.type, p3_tokens)) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, term_p2(), token.type);
        token = tokenizer->peek();
    }
    return node;
}

ASTNode* Parser::term_p4() {
    ASTNode* node = term_p3();
    auto token = tokenizer->peek();
    static std::set<Token::TokenType> p4_tokens = {
        Token::TokenType::OP_EQ,
        Token::TokenType::OP_NE,
        Token::TokenType::OP_GT,
        Token::TokenType::OP_GE,
        Token::TokenType::OP_LT,
        Token::TokenType::OP_LE,
    };
    while(Token::typeIn(token.type, p4_tokens)) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, term_p3(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::expr() {
    return term_p4();
}

// assignstmt : LET? VAR ASSIGN expr
AssignStmtNode* Parser::parseAssignStmt() {
    auto node = tokenizer->peek();
    if (node.type == Token::TokenType::LET) {
        tokenizer->eat(Token::TokenType::LET);
    }
    auto varNode = parseVar();
    tokenizer->eat(Token::TokenType::ASSIGN);
    ASTNode* expr_node = expr();
    return new AssignStmtNode(varNode, expr_node);
}
// gotostmt : GOTO NUM
GOTOStmtNode* Parser::parseGOTOStmt() {
    tokenizer->eat(Token::TokenType::GOTO);
    auto token = tokenizer->peek();
    if(token.type != Token::TokenType::NUM) {
        throw std::runtime_error("Should only GOTO number, but goto type " + Token::tk2Str(token.type));
    }
    tokenizer->eat(Token::TokenType::NUM);
    if (!token.value.has_value()) {
        throw std::runtime_error("GOTO: number token should not be null");
    }
    int line_no = str2Int(token.value.value());
    return new GOTOStmtNode(line_no);
}
// END
EndStmtNode* Parser::parseEndStmt() {
    tokenizer->eat(Token::TokenType::END);
    return new EndStmtNode();
}
// PRINT expr
PrintStmtNode* Parser::parsePrintStmt() {
    tokenizer->eat(Token::TokenType::PRINT);
    // TODO: support multi expr
    auto expr_node = expr();
    return new PrintStmtNode(expr_node);
}
// INPUT var
InputStmtNode* Parser::parseInputStmt() {
    tokenizer->eat(Token::TokenType::INPUT);
    auto varNode = parseVar();
    return new InputStmtNode(varNode);
}
// IF expr THEN NUM
IFStmtNode* Parser::parseIFStmt() {
    tokenizer->eat(Token::TokenType::IF);
    auto expr_node = expr();
    tokenizer->eat(Token::TokenType::THEN);
    auto num_node = parseNum();
    int line_no = num_node->getInt();
    return new IFStmtNode(expr_node, line_no);
}
// REM comment
RemStmtNode* Parser::parseRemStmt() {
    tokenizer->eat(Token::TokenType::REM);
    auto token = tokenizer->peek();
    tokenizer->eat(Token::TokenType::LITERAL);
    return new RemStmtNode(token.value.value());
}

// program : (stmt*) eof
// SHOULD BE reentrant
void Parser::parseProgram() {
    auto eof = tokenizer->peek();
    if (eof.type == Token::TokenType::TK_EOF) {
        print("[Warning]: tokenizer has been used before calling parseProgram\n");
        // has been parsed
        return;
    }
    auto token_lines = tokenizer->get_token_lines();
    auto size = token_lines.size();
    for(int line_idx = 0; line_idx < size; ++line_idx) {
        auto& [line_no, tokens] = token_lines[line_idx];
        // tokenizer->set_line_offset(line_idx);
        // tokenizer->set_inline_offset(0);
        // ATTENTION: empty line should be filtered in tokenizer
        // so there is no EOF check here
        ASTNode* stmt = nullptr;
        try {
            auto token = tokenizer->peek();
            if(token.type == Token::TokenType::END) {
                stmt = parseEndStmt();
            } else if(token.type == Token::TokenType::PRINT) {
                stmt = parsePrintStmt();
            } else if(token.type == Token::TokenType::INPUT) {
                stmt = parseInputStmt();
            } else if(token.type == Token::TokenType::IF) {
                stmt = parseIFStmt();
            } else if(token.type == Token::TokenType::GOTO) {
                stmt = parseGOTOStmt();
            } else if(token.type == Token::TokenType::LET) {
                stmt = parseAssignStmt();
            } else if(token.type == Token::TokenType::VAR){
                stmt = parseAssignStmt(); // A = A + 1, eg
            } else if (token.type == Token::TokenType::REM) {
                stmt = parseRemStmt();
            } else {
                stmt = expr();
            }
            stmts[line_no] = stmt;
        } catch (std::exception& e) {
            print("Failed to parse line: {}, index {}\n", line_no, line_idx);
            print("Error: {}\n", e.what());
            throw std::runtime_error("Failed to parse line: " + std::to_string(line_no));
        }
    }
}
using fmt::print;
using fmt::format;
void printSingleStmt(int line_no, ASTNode* stmt) {
    // TODO
    print("{}: {}\n", line_no, stmt->toString());
}
void Parser::printAST(ASTNode* root) const {
    printSingleStmt(0, root);
}

/*
 * line_no: -1 print all, else print specific line
 */
void Parser::printAST(int line_no) const {
    if(line_no == -1) {
        for(auto& [line_no, stmt]: stmts) {
            printSingleStmt(line_no, stmt);
        }
    } else {
        auto stmt_it = stmts.find(line_no);
        if(stmt_it == stmts.end()) {
            print("No stmt found for line: {}\n", line_no);

            print("valid lines:");
            for(const auto &[num, _]: stmts) {
                print("{} ", num);
            }
            print("\n");
            return;
        }
        printSingleStmt(line_no, stmt_it->second);
    }
}


