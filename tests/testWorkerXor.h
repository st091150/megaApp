#ifndef TESTWORKERXOR_H
#define TESTWORKERXOR_H

#include <QObject>
#include <QTest>

class TestWorkerXor : public QObject {
    Q_OBJECT

private slots:
    void testXor_basic8bytes();
    void testXor_nonMultipleOf8();
    void testXor_empty();
};

#endif // TESTWORKERXOR_H
