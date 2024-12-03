//
// Created by ayanami on 12/3/24.
//

#include "parser.h"
// factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN | variable
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
            throw std::runtime_error("Invalid string token: " + Token::tk2Str(token.type));
    }
    tokenizer->eat(token.type);
    if (!token.value.has_value()) {
            throw std::runtime_error("string token value should not be null");
    }
    return new StringNode(token.value.value());
}
VarNode* Parser::parseVar() {
    auto token = tokenizer->peek();
    if(token.type != Token::TokenType::VAR) {
        throw std::runtime_error("Invalid variable token: " + Token::tk2Str(token.type));
    }
    tokenizer->eat(token.type);
    return new VarNode(token.value.value());
}
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
    throw std::runtime_error("Invalid factor token: " + Token::tk2Str(token.type));
}


// term_p1 : factor (POW factor)*
// term_p2 : factor ((MUL | DIV | MOD) factor)*
// term_p3 : term_p2 ((PLUS | MINUS) term_p2)*
// term_p4 : term_p3 ((GT | EQ | LT | GE | LE | NE) term_p3)*
// expr : term_p4
ASTNode* Parser::term_p1() {
    ASTNode* node = factor();
    auto token = tokenizer->peek();
    while(token.type == Token::TokenType::OP_POW) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, factor(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::term_p2() {
    ASTNode* node = factor();
    auto token = tokenizer->peek();
    static std::set<Token::TokenType> p2_tokens = {
        Token::TokenType::OP_MUL,
        Token::TokenType::OP_DIV,
        Token::TokenType::OP_MOD
    };
    while(Token::typeIn(token.type, p2_tokens)) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, factor(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::term_p3() {
    ASTNode* node = factor();
    auto token = tokenizer->peek();
    static std::set<Token::TokenType> p2_tokens = {
        Token::TokenType::OP_ADD,
        Token::TokenType::OP_SUB,
    };
    while(Token::typeIn(token.type, p2_tokens)) {
        tokenizer->eat(token.type);
        node = new BinOpNode(node, factor(), token.type);
        token = tokenizer->peek();
    }
    return node;
}

ASTNode* Parser::term_p4() {
    ASTNode* node = factor();
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
        node = new BinOpNode(node, factor(), token.type);
        token = tokenizer->peek();
    }
    return node;
}
ASTNode* Parser::expr() {
    return term_p4();
}

// assignstmt : LET VAR ASSIGN expr
AssignStmtNode* Parser::parseAssignStmt() {
    tokenizer->eat(Token::TokenType::LET);
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
// program : (stmt*) eof
void Parser::parseProgram() {
    auto token_lines = tokenizer->get_token_lines();
    for(auto& [line_no, tokens]: token_lines) {
        tokenizer->set_line_offset(line_no);
        tokenizer->set_inline_offset(0);
        ASTNode* stmt = nullptr;
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
        } else {
            throw std::runtime_error("Invalid stmt starter: " + Token::tk2Str(token.type));
        }
        stmts[line_no] = stmt;
    }

}
using fmt::print;
using fmt::format;
void printSingleStmt(int line_no, ASTNode* stmt) {
    // TODO
    print("{}: {}\n", line_no, stmt->type());
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
            return;
        }
        printSingleStmt(line_no, stmt_it->second);
    }

}
void Interpreter::visit(ASTNode *root) override {
    if(root == nullptr) {
        return;
    }
    auto type = root->type();
    switch (type) {
    case ASTNodeType::AssignStmt:
        return visit_AssignStmtNode(dynamic_cast<AssignStmtNode *>(root));
    case ASTNodeType::GOTOStmt:
        return visit_GOTOStmtNode(dynamic_cast<GOTOStmtNode *>(root));
    case ASTNodeType::EndStmt:
        return visit_EndStmtNode(dynamic_cast<EndStmtNode *>(root));
    case ASTNodeType::PrintStmt:
        return visit_PrintStmtNode(dynamic_cast<PrintStmtNode *>(root));
    case ASTNodeType::InputStmt:
        return visit_InputStmtNode(dynamic_cast<InputStmtNode *>(root));
    case ASTNodeType::IFStmt:
        return visit_IFStmtNode(dynamic_cast<IFStmtNode *>(root));
    default:
        throw std::runtime_error(format("Should Only Visit Statements. visit: {}", root->type()));
    }
}

void Interpreter::interpret() {
    try {
        parser->parseProgram();
    } catch (std::exception& e) {
        print("Failed to parse program: {}\n", e.what());
        parser->printAST(-1);
        status.err_msg = e.what();
        return;
    }

    auto stmts = parser->getStmts();
    vector<int> line_nos;
    for(const auto [line_no, _]: stmts ) {
        line_nos.push_back(line_no);
    }
    if(line_nos.empty()) {
        return;
    }
    status.running = true;
    auto line_no_idx = 0;
    status.current_line = line_nos[line_no_idx];
    auto reset_idx = [&line_no_idx, line_nos](int current_line) {
        line_no_idx = 0;
        while(line_no_idx < line_nos.size() && line_nos[line_no_idx] != current_line) {
            line_no_idx++;
        }
    };
    while(status.running && !status.err_msg.has_value() && line_no_idx < line_nos.size()) {
        auto stmt = stmts[status.current_line];
        line_no_idx++;
        status.next_line = line_nos[line_no_idx];
        try {
            visit(stmt); // may change status.running, status.err_msg, status.next_line
        } catch (std::exception& e) {
            print("Failed to interpret stmt: {}\n", e.what());
            status.err_msg = e.what();
            return;
        }
        status.current_line = status.next_line;
        reset_idx(status.current_line);
        // normally end: idx == size
    }
}

