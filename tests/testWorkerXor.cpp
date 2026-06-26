#include "testWorkerXor.h"
#include "worker.h"

#include <QtTest>

QTEST_MAIN(TestWorkerXor)

void TestWorkerXor::testXor_empty() {
    Worker w;

    QByteArray data;
    uint64_t key = 0x0102030405060708ULL;
    w.wordXor(data.data(), data.size(), key);

    QCOMPARE(data.size(), 0);
}

void TestWorkerXor::testXor_basic8bytes() {
    Worker w;
    uint64_t key = 0x1122334455667788ULL;

    QByteArray data("qwertyui");
    QByteArray original = data;

    w.wordXor(data.data(), data.size(), key);
    w.wordXor(data.data(), data.size(), key);

    QCOMPARE(data, original);
}

void TestWorkerXor::testXor_nonMultipleOf8() {
    Worker w;

    uint64_t key = 0x0102030405060708ULL;

    QByteArray data("HelloFromMegaApp123"); // 19 % 8 = 3
    QByteArray original = data;

    w.wordXor(data.data(), data.size(), key);
    w.wordXor(data.data(), data.size(), key);
    QCOMPARE(data, original);
}
