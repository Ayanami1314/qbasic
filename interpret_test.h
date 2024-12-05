//
// Created by ayanami on 12/4/24.
//
#pragma once
#ifndef INTERPRET_TEST_H
#define INTERPRET_TEST_H


#include <QTest>
#include <QObject>

class interpret_test: public QObject{
    Q_OBJECT
private slots:
    void initTestCase();
    void testEvalExpr();
    void testEvalExprSpecial();
    void testEvalAssign();
    void testRunWithJmp();
    void testIO();
    void cleanupTestCase();
};



#endif //INTERPRET_TEST_H
