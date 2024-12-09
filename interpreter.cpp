//
// Created by ayanami on 12/4/24.
//

#include "interpreter.h"

using fmt::format;
void Interpreter::visit(ASTNode *root) {
    if(root == nullptr) {
        return;
    }
    auto type = root->type();
    // update var
    if (type == ASTNodeType::Var) {
        visit_Expr(root);
    }
    // const value node
    if (belongsDataNode(root->type())) {
        return;
    }
    switch (type) {
    // stmt
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
    // expr op
    case ASTNodeType::UnaryOp:
        return visit_UnaryOp(dynamic_cast<UnaryOpNode *>(root));
    case ASTNodeType::BinOp:
        return visit_BinOp(dynamic_cast<BinOpNode *>(root));
    default:
        string s = fmt::format("Invalid visit node type. visit: {}\n", ast2Str(root->type()));
        throw std::runtime_error(s);
    }
}
void Interpreter::interpret() {
    if(status.err_msg.has_value() || !parser || parser->getStmts().empty()) {
        print("Invalid status to interpret\n");
        return;
    }
    if(status.running) {
        print("Interpreter is already running\n");
        return;
    }
    if(status.current_line == -1 && status.next_line == 0) {
        // start from the first line
        status.next_line = parser->getStmts().begin()->first;
    } else {
        status.current_line = status.next_line; // Example: Resume, and pass the current line
    }

    status.running = true;
    for(const auto line: status.breakpoints) {
        print("Breakpoints: {}\n", line);
    }
    while(status.running && !status.err_msg.has_value() && !status.break_at(status.current_line)) {
        interpret_SingleStep();
        print("[DEBUG] Current line: {}\n", status.current_line);
        if(status.mode == ProgramMode::DEV || status.mode == ProgramMode::DEBUG) {
            print("[DEBUG] Current line: {}\n", status.current_line);
            print("[DEBUG] AST:\n");
            parser->printAST(status.current_line);
            print("[DEBUG] Env:\n");
            env->print();
            print("---\n");
        }
    }
    status.running = false;
    if(status.break_at(status.current_line)) {
        print("[DEBUG] Break at line: {}\n", status.current_line);
    }
}


/*
 * assert:
 * - status.running == true
 * - status.err_msg == nullopt
 * - has parsed
 * interpret: next line
 */
void Interpreter::interpret_SingleStep() {
    auto stmts = parser->getStmts();
    if(!status.running || status.err_msg.has_value() || stmts.empty()) {
        print("Invalid status to interpret\n");
        return;
    }
    if(!stmts.contains(status.next_line)) {
        print("Invalid current line: {}\n", status.next_line);
        status.err_msg = format("line {} no exist", status.next_line);
        return;
    }
    auto normal_next_it = std::next(stmts.find(status.next_line));
    int origin_current = status.current_line;
    try {
        status.current_line = status.next_line;
        visit(stmts[status.next_line]); // might change next_line
        // astOutput(stmts[status.next_line]->toString());
        auto strs = stmts[status.next_line]->toTabbedString();
        for(const auto& s: strs) {
            astOutput(s);
        }
    } catch (std::exception& e) {
        print("Failed to interpret stmt: {}\n", e.what());
        status.err_msg = e.what();
        status.running = false;
        status.current_line = origin_current; // recover
        throw e; // pass to upper level
    }
    if(!status.running || status.err_msg.has_value()) {
        return;
    }


    // next line changed
    if(status.next_line != status.current_line) {
        return;
    }

    if(normal_next_it == stmts.end()) {
        status.next_line = -1; // will end in next call
        return;
    }
    // normal next line
    status.next_line = normal_next_it->first;
}