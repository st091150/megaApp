#ifndef LOGGER_H
#define LOGGER_H

#include "loggerConfig.h"

#include <QCoreApplication>
#include <QFile>
#include <QSharedPointer>
#include <QString>
#include <QTextEdit>
#include <QTextStream>


class Logger {
public:
  enum ELogLevel { Info, Warning, Error };

  enum EOutputMode { FileSystemOnly, UIOnly, Both };

public:
  explicit Logger(QTextEdit *uiLogTextEditObj = nullptr,
                  const QString &baseLogPath =
                      QCoreApplication::applicationDirPath() + BASE_LOG_FOLDER,
                  int maxFileSize = MAX_FILE_SIZE,
                  int maxFileCount = MAX_FILE_COUNT);
  ~Logger();

  void setUiLogObj(QTextEdit *uiLogTextEditObj);
  void setBasePath(const QString &path);
  void setRotationPolicy(int maxSizeBytes, int maxFileCount);
  void setOutputMode(EOutputMode mode);

  EOutputMode outputMode() const;

public:
  void log(const QString &msg, ELogLevel level = Info);
  inline void info(const QString &msg) { log(msg, Info); }
  inline void warning(const QString &msg) { log(msg, Warning); }
  inline void error(const QString &msg) { log(msg, Error); }

private:
  void openCurrentFile(QIODevice::OpenMode mode);

  void rotateLogs();

  QString currentFileName() const;
  QString levelToString(ELogLevel level) const;
  QString fileTimestamp() const;
  QString uiTimestamp() const;

private:
  QTextEdit *_uiTextEditLog;
  EOutputMode _outputMode;
  QString _basePath;
  int _maxFileSize;
  int _maxFileCount;
  int _currentIndex;
  QFile _logFile;
  QTextStream _logStream;
};

#endif // LOGGER_H
