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
    // auto tokenLines = test_tokenizer->read_lines(src);
    auto table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer);
    // test_parser->parseProgram();
    auto env = std::make_shared<Env>(table);
    auto interpreter = std::make_shared<Interpreter>(test_parser, env, ProgramMode::DEV);
    auto p = Token::programFromlines(src);
    interpreter->loadProgram(std::move(p));
    return interpreter;
}
std::shared_ptr<Interpreter> buildInterpreter(const std::filesystem::path& fpath) {
    auto test_tokenizer = std::make_shared<Token::Tokenizer>();;
    // auto tokenLines = test_tokenizer->read_lines(src);
    auto table = std::make_shared<SymbolTable>();
    auto test_parser = std::make_shared<Parser>(test_tokenizer);
    // test_parser->parseProgram();
    auto env = std::make_shared<Env>(table);
    auto interpreter = std::make_shared<Interpreter>(test_parser, env);
    interpreter->loadFile(fpath);
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
    // std::istringstream simulated_input("2\n4\n");
    // auto origin_buf = std::cin.rdbuf();
    // std::cin.rdbuf(simulated_input.rdbuf()); // mock input
    interpreter->input("2\n4\n"); // 简单化:单线程测试，interpret无回调和异步,提前备好输入
    interpreter->interpret();
    // std::cin.rdbuf(origin_buf);

    QVERIFY2(interpreter->getEnv()->symbol_table->get<int>("res") == 6, "Failed to evaluate assignment");
}

void interpret_test::testSumOfOneToN() {
    auto src = "./programs/sum_of_1ton.bas";

    auto fpath = std::filesystem::path(src);
    auto interpreter = buildInterpreter(fpath);
    interpreter->input("10\n");
    interpreter->interpret();
    QVERIFY2(interpreter->getEnv()->symbol_table->get<int>("sum") == 55, "Failed to evaluate assignment");
}
void interpret_test::testFibonacci() {
    auto src = "./programs/fib.bas";

    auto fpath = std::filesystem::path(src);
    auto interpreter = buildInterpreter(fpath);
    interpreter->interpret();
    auto t = interpreter->getEnv()->symbol_table;
    auto n1 = t->get<int>("n1");
    auto n2 = t->get<int>("n2");
    auto n3 = t->get<int>("n3");
    QVERIFY2(n1 > 10000, "Failed to evaluate assignment");
    QVERIFY2(n2 > 10000, "Failed to evaluate assignment");
    QVERIFY2(n3 > 10000, "Failed to evaluate assignment");
    print("n1: {}, n2: {}, n3: {}\n", n1.value(), n2.value(), n3.value());
}
void interpret_test::testPrime() {
    auto src = "./programs/is_prime.bas";

    auto fpath = std::filesystem::path(src);
    auto interpreter = buildInterpreter(fpath);
    typedef struct TestPrime {
        int n;
        int is_prime;
    } TestPrime;
    vector<TestPrime> tests = {{2, 1}, {5, 1}, {7, 1},
        {10, 0}, {20, 0}, {31, 1},  {67, 1},
    {100, 0}};
    for (const auto& t: tests) {
        interpreter->input(format("{}\n", t.n));
        interpreter->interpret();
        auto is_prime = interpreter->getEnv()->symbol_table->get<int>("is_prime");
        QVERIFY2(is_prime.value() == t.is_prime, fmt::format("test {}: expect {} != actual {}",
            t.n, t.is_prime, is_prime.value()).c_str());
        interpreter->reload();
    }
}
void interpret_test::testFactorial() {
    auto src = "./programs/factorial.bas";

    auto fpath = std::filesystem::path(src);
    auto interpreter = buildInterpreter(fpath);
    interpreter->input("5\n");
    interpreter->interpret();
    // 5! = 120
    QVERIFY2(interpreter->getEnv()->symbol_table->get<int>("fact") == 120,
        "Failed to evaluate assignment");
}

void interpret_test::cleanupTestCase() {
    qDebug() <<"Cleanup test case\n";

}