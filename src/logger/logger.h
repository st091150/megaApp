#ifndef LOGGER_H
#define LOGGER_H

#include "loggerConfig.h"

#include <QString>
#include <QFile>
#include <QSharedPointer>
#include <QTextStream>
#include <QTextEdit>
#include <QCoreApplication>

class Logger {
public:
    enum LogLevel {
        Info,
        Warning,
        Error
    };

    enum OutputMode {
        FileSystemOnly,
        UIOnly,
        Both
    };

public:
    explicit Logger(QTextEdit* uiLogTextEditObj = nullptr,
                    const QString& baseLogPath = QCoreApplication::applicationDirPath() + BASE_LOG_FOLDER,
                    int maxFileSize = MAX_FILE_SIZE,
                    int maxFileCount = MAX_FILE_COUNT);
    ~Logger();

    void setUiLogObj(QTextEdit* uiLogTextEditObj);
    void setBasePath(const QString& path);
    void setRotationPolicy(int maxSizeBytes, int maxFileCount);
    void setOutputMode(OutputMode mode);

    OutputMode outputMode() const;

public:
    void log(const QString& msg, LogLevel level = Info);
    inline void info(const QString& msg) { log(msg, Info); }
    inline void warning(const QString& msg) { log(msg, Warning); }
    inline void error(const QString& msg) { log(msg, Error); }

private:
    void openCurrentFile(QIODevice::OpenMode mode);
    void rotateLogs();
    QString currentFileName() const;
    QString levelToString(LogLevel level) const;
    QString fileTimestamp() const;
    QString uiTimestamp() const;

private:
    QTextEdit* _uiTextEditLog;
    OutputMode _outputMode;
    QString _basePath;
    int _maxFileSize;
    int _maxFileCount;
    int _currentIndex;
    QFile _logFile;
    QTextStream _logStream;

};

#endif // LOGGER_H
