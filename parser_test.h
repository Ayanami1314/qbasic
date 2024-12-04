//
// Created by ayanami on 12/4/24.
//

#pragma once

#ifndef PARSER_TEST_H
#define PARSER_TEST_H


#include <QTest>
#include <QObject>
class parser_test: public QObject{
    Q_OBJECT
private slots:
    void initTestCase();
    void testParseNum();
    void testParseOp();
    void testParseExpr();
    void testParseAssign();
    void testParseIfAndGoto();
    void testIO();
    void cleanupTestCase();
};



#endif //PARSER_TEST_H
