#include "testLogger.h"
#include "logger.h"
#include "loggerConfig.h"

#include <QTextEdit>
#include <QDir>
#include <QFile>
#include <QTest>


QTEST_MAIN(LoggerTest)

static QString testLogDir = QDir::temp().filePath("/test_logs");

void LoggerTest::init() {
    QDir().mkpath(testLogDir);
}

void LoggerTest::cleanup() {
    QDir dir(testLogDir);
    dir.removeRecursively();
}

void LoggerTest::testFileLogging() {
    Logger logger(nullptr, testLogDir, 10 * 1024, 3);

    logger.setOutputMode(Logger::FileSystemOnly);
    logger.info("Hello file");

    QTest::qWait(50);

    QDir dir(testLogDir);

    QString file = testLogDir + QString("/%1.0.log").arg(BASE_LOG_FILE_NAME);

    QFile f(file);
    QVERIFY(f.open(QIODevice::ReadOnly));

    QString content = f.readAll();
    QVERIFY(content.contains("Hello file"));
}

void LoggerTest::testUiLogging() {
    QTextEdit edit;

    Logger logger(&edit);
    logger.setOutputMode(Logger::UIOnly);

    logger.info("Hello UI");

    QString text = edit.toPlainText();
    QVERIFY(text.contains("Hello UI"));
}

void LoggerTest::testRotation() {
    Logger logger(nullptr, testLogDir, 50, 2);
    logger.setOutputMode(Logger::FileSystemOnly);
    for (int i = 0; i < 20; ++i) {
        logger.info("AAAAAAAAAAAAAAAAAAAAAAAA");
    }
    QDir dir(testLogDir);
    QStringList files = dir.entryList(QStringList() << "*.log");
    QVERIFY(files.size() <= 2);
}

void LoggerTest::testOutputModes() {
    QTextEdit edit;

    Logger logger(&edit, testLogDir);
    logger.setOutputMode(Logger::Both);
    logger.info("Both mode test");

    QFile f(testLogDir + QString("/%1.0.log").arg(BASE_LOG_FILE_NAME));

    QVERIFY(f.exists());
    QVERIFY(edit.toPlainText().contains("Both mode test"));
}
