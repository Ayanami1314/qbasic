//
// Created by ayanami on 12/3/24.
//

#include "tokenizer_test.h"
#include "tokenizer.h"
#include <QTest>
#include <fmt/core.h>

using std::vector;
using std::string;
using fmt::print;
void tokenizer_test::initTestCase() {
    print("Init test case\n");
}

void tokenizer_test::basicNumTest() {
    print("Basic num test\n");
    std::string data = "10 1 2 3 4 5 6 7 8 9 10";
    vector<string> lines = {data};
    auto tokenLines = test_tokenizer.read_lines(lines);
    QVERIFY2(tokenLines.size() == 1, "Failed to tokenize data");
    auto [lineNo, tokens] = tokenLines[0];
    QVERIFY(lineNo == 10);
    QVERIFY2(tokens.size() == 10, "Failed to tokenize data");
    for (int i = 0; i < 10; i++) {
        QVERIFY2(tokens[i].type == Token::TokenType::NUM,
                 "Failed to tokenize data");
        QVERIFY2(tokens[i].value == std::to_string(i + 1),
                 "Failed to tokenize data");
    }
}
void tokenizer_test::basicIFTest() {
    print("Basic if test\n");
    std::string if_st = "10 IF 1 > 2 THEN 3";
    vector<string> lines = {if_st};
    auto tokenLines = test_tokenizer.read_lines(lines);
    QVERIFY2(tokenLines.size() == 1, "Failed to tokenize data");
    auto [lineNo, tokens] = tokenLines[0];
    QVERIFY(lineNo == 10);
    QVERIFY2(tokens.size() == 6, "Failed to tokenize data");
    QVERIFY(tokens[0].type == Token::TokenType::IF);
    QVERIFY(tokens[1].type == Token::TokenType::NUM && tokens[1].value == "1");
    QVERIFY(tokens[2].type == Token::TokenType::OP_GT);
    QVERIFY(tokens[3].type == Token::TokenType::NUM && tokens[3].value == "2");
    QVERIFY(tokens[4].type == Token::TokenType::THEN);
    QVERIFY(tokens[5].type == Token::TokenType::NUM && tokens[5].value == "3");

}

void tokenizer_test::basicOpTest() {
        print("Basic Op test\n");
        std::string data = "10 + - * / MOD **";
        vector<string> lines = {data};
        auto tokenLines = test_tokenizer.read_lines(lines);
        QVERIFY2(tokenLines.size() == 1, "Failed to tokenize data");
        auto [lineNo, tokens] = tokenLines[0];
        QVERIFY(lineNo == 10);
        QVERIFY2(tokens.size() == 6, "Failed to tokenize data");
        QVERIFY(tokens[0].type == Token::TokenType::OP_ADD);
        QVERIFY(tokens[1].type == Token::TokenType::OP_SUB);
        QVERIFY(tokens[2].type == Token::TokenType::OP_MUL);
        QVERIFY(tokens[3].type == Token::TokenType::OP_DIV);
        QVERIFY(tokens[4].type == Token::TokenType::OP_MOD);
        QVERIFY(tokens[5].type == Token::TokenType::OP_POW);
}
void tokenizer_test::basicVarTest() {
    print("Basic Var test\n");
    vector<string> lines;
    lines.emplace_back("10 LET A = 1");
    lines.emplace_back("20 LET bb3 = 2");
    lines.emplace_back("30 LET _c = 3");
    lines.emplace_back("40 LET _ = 4");
    std::string mess = "wddw1221438dw_sdsa_986dd_";
    lines.emplace_back(fmt::to_string(fmt::format("50 LET {} = 4", mess)));
    auto tokenLines = test_tokenizer.read_lines(lines);
    QVERIFY2(tokenLines.size() == lines.size(), "Failed to tokenize data");
    for (int i = 0; i < 5; i++) {
        auto [lineNo, tokens] = tokenLines[i];
        QVERIFY(lineNo == 10 * (i + 1));
        QVERIFY2(tokens.size() == 4, "Failed to tokenize data");
        QVERIFY(tokens[0].type == Token::TokenType::LET);
        QVERIFY(tokens[1].type == Token::TokenType::VAR);
        QVERIFY(tokens[2].type == Token::TokenType::ASSIGN);
        QVERIFY(tokens[3].type == Token::TokenType::NUM);
    }
}
void tokenizer_test::basicExprTest() {
    print("Basic Expr test\n");
    vector<string> lines;
    lines.emplace_back("10 1 + 2");
    lines.emplace_back("20 1 * 2 / 3");
    lines.emplace_back("30 1 - 2 * 3");
    lines.emplace_back("40 1 / (2 ** 3)");
    lines.emplace_back("50 (1 MOD (2 + 5)) * 7");
    auto tokenLines = test_tokenizer.read_lines(lines);
    QVERIFY2(tokenLines.size() == lines.size(), "Failed to tokenize data");
    auto [lineNo, tokens] = tokenLines[0];
    QVERIFY(lineNo == 10);
    QVERIFY2(tokens.size() == 3, "Failed to tokenize data");
    QVERIFY(tokens[0].type == Token::TokenType::NUM);
    QVERIFY(tokens[1].type == Token::TokenType::OP_ADD);
    QVERIFY(tokens[2].type == Token::TokenType::NUM);

    auto [lineNo1, tokens1] = tokenLines[1];
    QVERIFY(lineNo1 == 20);
    QVERIFY(tokens1.size() == 5);
    QVERIFY(tokens1[0].type == Token::TokenType::NUM);
    QVERIFY(tokens1[1].type == Token::TokenType::OP_MUL);
    QVERIFY(tokens1[2].type == Token::TokenType::NUM);
    QVERIFY(tokens1[3].type == Token::TokenType::OP_DIV);
    QVERIFY(tokens1[4].type == Token::TokenType::NUM);

    auto [lineNo2, tokens2] = tokenLines[2];
    QVERIFY(lineNo2 == 30);
    QVERIFY(tokens2.size() == 5);
    QVERIFY(tokens2[0].type == Token::TokenType::NUM);
    QVERIFY(tokens2[1].type == Token::TokenType::OP_SUB);
    QVERIFY(tokens2[2].type == Token::TokenType::NUM);
    QVERIFY(tokens2[3].type == Token::TokenType::OP_MUL);
    QVERIFY(tokens2[4].type == Token::TokenType::NUM);

    auto [lineNo3, tokens3] = tokenLines[3];
    QVERIFY(lineNo3 == 40);
    QVERIFY(tokens3.size() == 7);
    QVERIFY(tokens3[0].type == Token::TokenType::NUM);
    QVERIFY(tokens3[1].type == Token::TokenType::OP_DIV);
    QVERIFY(tokens3[2].type == Token::TokenType::LPAREN);
    QVERIFY(tokens3[3].type == Token::TokenType::NUM);
    QVERIFY(tokens3[4].type == Token::TokenType::OP_POW);
    QVERIFY(tokens3[5].type == Token::TokenType::NUM);
    QVERIFY(tokens3[6].type == Token::TokenType::RPAREN);

    auto [lineNo4, tokens4] = tokenLines[4];
    QVERIFY(lineNo4 == 50);
    QVERIFY(tokens4.size() == 11);
    // (1 MOD (2 + 5)) * 7
    QVERIFY(tokens4[0].type == Token::TokenType::LPAREN);
    QVERIFY(tokens4[1].type == Token::TokenType::NUM);
    QVERIFY(tokens4[2].type == Token::TokenType::OP_MOD);
    QVERIFY(tokens4[3].type == Token::TokenType::LPAREN);
    QVERIFY(tokens4[4].type == Token::TokenType::NUM);
    QVERIFY(tokens4[5].type == Token::TokenType::OP_ADD);
    QVERIFY(tokens4[6].type == Token::TokenType::NUM);
    QVERIFY(tokens4[7].type == Token::TokenType::RPAREN);
    QVERIFY(tokens4[8].type == Token::TokenType::RPAREN);
    QVERIFY(tokens4[9].type == Token::TokenType::OP_MUL);
    QVERIFY(tokens4[10].type == Token::TokenType::NUM);

}
void tokenizer_test::basicLineTest() {

}
void tokenizer_test::multiLineTest1() {

}


void tokenizer_test::cleanupTestCase() {

}
