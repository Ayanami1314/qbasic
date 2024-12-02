//
// Created by ayanami on 12/3/24.
//
//
// Created by ayanami on 9/19/24.
//
#include <QTest>
#include "tokenizer_test.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    tokenizer_test test;
    return QTest::qExec(&test, argc, argv);
}
