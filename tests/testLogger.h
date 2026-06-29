#ifndef TESTLOGGER_H
#define TESTLOGGER_H

#include <QObject>

class LoggerTest : public QObject {
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  void testFileLogging();
  void testUiLogging();
  void testRotation();
  void testOutputModes();
};

#endif // TESTLOGGER_H
