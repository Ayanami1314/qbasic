//
// Created by ayanami on 12/4/24.
//

#include "interpret_test.h"
#include "tokenizer.h"
#include "parser.h"
#include "util.h"
#include "interpreter.h"
using std::vector;
using std::string;
using fmt::format;
std::shared_ptr<Interpreter> buildInterpreter(const vector<string>& src) {
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();;
    auto tokenLines = test_tokenizer->read_lines(src);
    auto table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer, table);
    test_parser->parseProgram();
    auto env = std::make_shared<Env>(table);
    auto interpreter = std::make_shared<Interpreter>(test_parser, env);
    return interpreter;
}
void interpret_test::initTestCase() {
    qDebug() <<"Init test case\n";
}
void interpret_test::testEvalExpr() {
    vector<string> src{
        "10 1+2",
        "20 2/3",
        "30 3**5",
        "40 88-99",
        "50 1*-2",
        "60 1+2*3",
        "70 6 MOD (2+3)",
        "80 ((1))",
    };
    auto interpreter = buildInterpreter(src);
    auto parser = interpreter->getParser();
    auto stmts = parser->getStmts();
    QVERIFY2(stmts.size() == 8, "Failed to parse stmts");
    vector<int> ref_results = {3, 0, 243, -11, -2, 7, 1, 1};
    for(int i=0;i<8;++i) {
        try {
            auto expr = stmts[(i+1)*10];
            parser->printAST(expr);
            interpreter->visit_Expr(expr);
            qDebug() << std::any_cast<int>(expr->getVal()) << '\n';
            QVERIFY2(std::any_cast<int>(expr->getVal()) == ref_results[i], "Failed to evaluate expression");
        } catch (std::exception& e) {
            QVERIFY2(1 == 0, e.what());
            return;
        }
    }
}
void interpret_test::testEvalExprSpecial() {
    vector<string> src{
        "10 2 ** 2 ** 3",
        "20 5 MOD -3",
    };
    auto interpreter = buildInterpreter(src);
    auto parser = interpreter->getParser();
    auto stmts = parser->getStmts();
    // 2^8 == 256
    QVERIFY2(stmts.size() == src.size(), "Failed to parse stmts");
    vector<int> ref_results = {static_cast<int>(std::pow(2, 8)), -1};
    for(int i=0;i<src.size();++i) {
        try {
            auto expr = stmts[(i+1)*10];
            parser->printAST(expr);
            interpreter->visit_Expr(expr);
            qDebug() << std::any_cast<int>(expr->getVal()) << '\n';
            QVERIFY2(std::any_cast<int>(expr->getVal()) == ref_results[i], "Failed to evaluate expression");
        } catch (std::exception& e) {
            QVERIFY2(1 == 0, e.what());
            return;
        }
    }
}
void interpret_test::testEvalAssign() {
    vector<string> lines = {
        "10 LET A = 1",
        "20 LET _B = 2",
        "30 LET C = A + _B",
        "40 LET D = A * _B - 3",
        "50 D = D + 1",
    };
    auto interpreter = buildInterpreter(lines);
    auto parser = interpreter->getParser();
    auto stmts = parser->getStmts();
    QVERIFY2(stmts.size() == 5, "Failed to parse stmts");
    vector<int> ref_results = {1, 2, 3, -1, 0};
    for(int i=0;i<5;++i) {
        auto stmt = stmts[(i+1)*10];
        QVERIFY(stmt->type() == ASTNodeType::AssignStmt);
        interpreter->visit_AssignStmtNode(dynamic_cast<AssignStmtNode *>(stmt));
        QVERIFY2(std::any_cast<int>(stmt->getVal()) == ref_results[i], "Failed to evaluate assignment");
    }
    auto a = interpreter->getEnv()->symbol_table->get<int>("A");
    QVERIFY2(a == 1, format("wrong A {}", a.value()).c_str());
    auto b = interpreter->getEnv()->symbol_table->get<int>("_B");
    QVERIFY2(b == 2, format("wrong B {}", b.value()).c_str());
    auto c = interpreter->getEnv()->symbol_table->get<int>("C");
    QVERIFY2(c == 3, format("wrong C {}", c.value()).c_str());
    auto d = interpreter->getEnv()->symbol_table->get<int>("D");
    QVERIFY2(d == 0, format("wrong D {}", d.value()).c_str());
}
void interpret_test::testRunWithJmp() {
    vector<string> lines = {
        "10 IF 1 THEN 40",
        "20 LET A = 20",
        "30 END",
        "40 LET A = 40",
    };

    auto interpreter = buildInterpreter(lines);
    auto parser = interpreter->getParser();
    auto stmts = parser->getStmts();
    QVERIFY2(stmts.size() == 4, "Failed to parse stmts");
    interpreter->interpret();
    QVERIFY2(interpreter->getEnv()->symbol_table->get<int>("A") == 40, "Failed to evaluate assignment");
}
void interpret_test::testIO() {
    vector<string> lines = {
        "10 PRINT 3",
        "20 INPUT n1",
        "30 INPUT n2",
        "40 LET res = n1 + n2",
        "50 PRINT res",
    };
    auto interpreter = buildInterpreter(lines);
    auto parser = interpreter->getParser();
    std::istringstream simulated_input("2\n4\n");
    auto origin_buf = std::cin.rdbuf();
    std::cin.rdbuf(simulated_input.rdbuf()); // mock input
    interpreter->interpret();
    std::cin.rdbuf(origin_buf);
}
void interpret_test::cleanupTestCase() {
    qDebug() <<"Cleanup test case\n";

}