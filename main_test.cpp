//
// Created by ayanami on 12/3/24.
//
//
// Created by ayanami on 9/19/24.
//
#include <QTest>

#include "parser_test.h"
#include "tokenizer_test.h"
#include "interpret_test.h"

int main(int argc, char *argv[]) {
    tokenizer_test test_lexer;
    parser_test test_parser;
    interpret_test test_interpret;
    QTest::qExec(&test_lexer, argc, argv);
    QTest::qExec(&test_parser, argc, argv);
    QTest::qExec(&test_interpret, argc, argv);
}
