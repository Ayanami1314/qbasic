//
// Created by ayanami on 12/4/24.
//

#include "parser_test.h"
#include <QtCore>
#include "tokenizer.h"
#include "parser.h"
#include "util.h"
using std::vector;
using std::string;
void parser_test::initTestCase() {
    qDebug() <<"Init test case\n";
}


void parser_test::testParseNum() {
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();
    vector<string> lines = {"10 1", "20 2", "30 232378", "40 0", "50 123456789"};
    auto tokenLines = test_tokenizer->read_lines(lines);
    auto test_table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, test_table);
    auto ref_nums = vector<int>{1, 2, 232378, 0, 123456789};
    for(int i=0; i<5; i++) {
        auto [lineNo, tokens] = tokenLines[i];
        QVERIFY2(tokens.size() == 1, "Failed to tokenize data");
        string what = "get type " + Token::tk2Str(tokens[i].type);
        QVERIFY2(tokens[0].type == Token::TokenType::NUM, what.c_str());
        auto num_i = test_parser->parseNum();
        QVERIFY2(num_i->getInt() == ref_nums[i], "Failed to parse number");
    }
}
void parser_test::testParseOp() {

    vector<string> lines = {
        "10 1+2",
        "20 2/3",
        "30 3**5",
        "40 88-99",
        "50 1*-2",
        "60 1+2*3",
        "70 6 MOD (2+3)",
        "80 ((1))",
    };
    // 问题: 88-99\n-1*2 算单行还是多行?
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();
    auto tokenLines = test_tokenizer->read_lines(lines);
    auto test_table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, test_table);
    auto expr_1 = test_parser->expr();
    string what = "get type " + ast2Str(expr_1->type());
    QVERIFY2(expr_1->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_1 = dynamic_cast<BinOpNode*>(expr_1);
    QVERIFY2(bin_op_1->getOp() == Token::TokenType::OP_ADD, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_1->getLeft())->getInt() == 1, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_1->getRight())->getInt() == 2, "Failed to parse expr");

    auto expr_2 = test_parser->expr();
    what = "get type " + ast2Str(expr_2->type());
    QVERIFY2(expr_2->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_2 = dynamic_cast<BinOpNode*>(expr_2);
    what = "get op " + Token::tk2Str(bin_op_2->getOp());
    QVERIFY2(bin_op_2->getOp() == Token::TokenType::OP_DIV, what.c_str());
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_2->getLeft())->getInt() == 2, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_2->getRight())->getInt() == 3, "Failed to parse expr");


    auto expr_3 = test_parser->expr();
    what = "get type " + ast2Str(expr_3->type());
    QVERIFY2(expr_3->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_3 = dynamic_cast<BinOpNode*>(expr_3);
    QVERIFY2(bin_op_3->getOp() == Token::TokenType::OP_POW, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_3->getLeft())->getInt() == 3, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_3->getRight())->getInt() == 5, "Failed to parse expr");

    auto expr_4 = test_parser->expr();
    test_parser->printAST(expr_4);
    what = "get type " + ast2Str(expr_4->type());
    QVERIFY2(expr_4->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_4 = dynamic_cast<BinOpNode*>(expr_4);
    QVERIFY2(bin_op_4->getOp() == Token::TokenType::OP_SUB, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_4->getLeft())->getInt() == 88, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_4->getRight())->getInt() == 99, "Failed to parse expr");

    auto expr_5 = test_parser->expr();

    what = "get type " + ast2Str(expr_5->type());
    QVERIFY2(expr_5->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_5 = dynamic_cast<BinOpNode*>(expr_5);
    QVERIFY2(bin_op_5->getOp() == Token::TokenType::OP_MUL, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_5->getLeft())->getInt() == 1, "Failed to parse expr");
    auto right = dynamic_cast<UnaryOpNode*>(bin_op_5->getRight());
    QVERIFY2(right->getOp() == Token::TokenType::OP_SUB, "Failed to parse expr");
    auto num = dynamic_cast<NumNode*>(right->getExpr());
    QVERIFY2(num->getInt() == 2, "Failed to parse expr");

    auto expr_6 = test_parser->expr();
    what = "get type " + ast2Str(expr_6->type());
    QVERIFY2(expr_6->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_6 = dynamic_cast<BinOpNode*>(expr_6);
    QVERIFY2(bin_op_6->getOp() == Token::TokenType::OP_ADD, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_6->getLeft())->getInt() == 1, "Failed to parse expr");
    QVERIFY2(bin_op_6->type() == ASTNodeType::BinOp, "Failed to parse expr");
    auto bin_op_6_2 = dynamic_cast<BinOpNode*>(bin_op_6->getRight());
    QVERIFY2(bin_op_6_2->getOp() == Token::TokenType::OP_MUL, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_6_2->getLeft())->getInt() == 2, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_6_2->getRight())->getInt() == 3, "Failed to parse expr");


    auto expr_7 = test_parser->expr();
    what = "get type " + ast2Str(expr_7->type());
    QVERIFY2(expr_7->type() == ASTNodeType::BinOp, what.c_str());
    auto bin_op_7 = dynamic_cast<BinOpNode*>(expr_7);
    QVERIFY2(bin_op_7->getOp() == Token::TokenType::OP_MOD, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_7->getLeft())->getInt() == 6, "Failed to parse expr");
    QVERIFY2(bin_op_7->type() == ASTNodeType::BinOp, "Failed to parse expr");
    auto bin_op_7_2 = dynamic_cast<BinOpNode*>(bin_op_7->getRight());
    QVERIFY2(bin_op_7_2->getOp() == Token::TokenType::OP_ADD, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_7_2->getLeft())->getInt() == 2, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(bin_op_7_2->getRight())->getInt() == 3, "Failed to parse expr");

    auto expr_8 = test_parser->expr();
    QVERIFY2(expr_8->type() == ASTNodeType::Num, "Failed to parse expr");
    QVERIFY2(dynamic_cast<NumNode*>(expr_8)->getInt() == 1, "Failed to parse expr");
    // TODO peek EOF

}
void parser_test::testParseExpr() {

}
void parser_test::testParseAssign() {
    vector<string> lines = {
        "10 LET A = 1",
        "20 LET _B = 2",
        "30 LET C = A + B",
        "40 LET D = A * B - 3",
        "50 D = D + 1",
    };
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();
    auto tokenLines = test_tokenizer->read_lines(lines);
    auto test_table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, test_table);
    for (int i=0; i<5; i++) {
        auto node = test_parser->parseAssignStmt();
        test_parser->printAST(node);
        QVERIFY2(node->type() == ASTNodeType::AssignStmt, "Failed to parse assign statement");
    }
}
void parser_test::testParseIfAndGoto() {
    vector<string> lines = {
        "1 LET A = 1",
        "2 LET B = 2",
        "10 IF A > B THEN 20",
        "20 IF A < B THEN 30",
        "30 IF A == B THEN 40",
        "40 IF A != B THEN 50",
        "50 IF A >= B THEN 60",
        "60 IF A <= B THEN 70",
        "70 IF 1 THEN 10",
        "80 GOTO 10",
    };
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();
    auto tokenLines = test_tokenizer->read_lines(lines);
    auto test_table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, test_table);
    test_parser->parseProgram();
    auto stmts = test_parser->getStmts();
    QVERIFY2(stmts.size() == 10, "Failed to parse program");
    for(int i=1;i<=8;++i) {
        QVERIFY2(stmts.contains(i*10), "Failed to parse program");
    }
    vector<Token::TokenType> ref_ops = {
        Token::TokenType::OP_GT,
        Token::TokenType::OP_LT,
        Token::TokenType::OP_EQ,
        Token::TokenType::OP_NE,
        Token::TokenType::OP_GE,
        Token::TokenType::OP_LE,
    };
    for (int i=1; i<=6; i++) {
        QVERIFY2(stmts[i * 10]->type() == ASTNodeType::IFStmt, "Failed to parse if statement");
        auto if_stmt = dynamic_cast<IFStmtNode*>(stmts[i * 10]);
        QVERIFY2(if_stmt->getCond()->type() == ASTNodeType::BinOp, "Failed to parse if statement");
        auto bin_op = dynamic_cast<BinOpNode*>(if_stmt->getCond());
        QVERIFY2(bin_op->getOp() == ref_ops[i-1], "Failed to parse if statement");
    }
    QVERIFY2(stmts[70]->type() == ASTNodeType::IFStmt, "Failed to parse if statement");
    auto if_stmt = dynamic_cast<IFStmtNode*>(stmts[70]);
    QVERIFY2(if_stmt->getCond()->type() == ASTNodeType::Num, "Failed to parse if statement");
    QVERIFY2(stmts[80]->type() == ASTNodeType::GOTOStmt, "Failed to parse goto statement");
}
void parser_test::testIO() {
    vector<string> lines = {
        "10 PRINT 3",
        "20 INPUT n1",
        "30 INPUT n2",
        "40 LET res = n1 + n2",
        "50 PRINT res",
    };
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();
    auto tokenLines = test_tokenizer->read_lines(lines);
    auto test_table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, test_table);

    test_parser->parseProgram();
    auto stmts = test_parser->getStmts();
    QVERIFY2(stmts.size() == 5, "Failed to parse program");
    for(int i=1;i<=5;++i) {
        QVERIFY2(stmts.contains(i*10), "Failed to parse program");
    }
    QVERIFY2(stmts[10]->type() == ASTNodeType::PrintStmt, "Failed to parse program");
    QVERIFY2(stmts[20]->type() == ASTNodeType::InputStmt, "Failed to parse program");
    QVERIFY2(stmts[30]->type() == ASTNodeType::InputStmt, "Failed to parse program");
    QVERIFY2(stmts[40]->type() == ASTNodeType::AssignStmt, "Failed to parse program");
    QVERIFY2(stmts[50]->type() == ASTNodeType::PrintStmt, "Failed to parse program");
}

void parser_test::cleanupTestCase() {
    qDebug() << "Cleanup test case\n";
}
