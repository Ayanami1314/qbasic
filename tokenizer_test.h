//
// Created by ayanami on 12/3/24.
//

#pragma once
#ifndef TOKENIZER_TEST_H
#define TOKENIZER_TEST_H


#include <QObject>
#include "tokenizer.h"
class tokenizer_test : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void basicNumTest();
    void basicOpTest();
    void basicVarTest();
    void basicExprTest();
    void basicIFTest();
    void basicLineTest();
    void multiLineTest1();
    void cleanupTestCase();

private:
    Token::Tokenizer test_tokenizer;
};



#endif //TOKENIZER_TEST_H
