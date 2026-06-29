#include "logger.h"

#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QScrollBar>

Logger::Logger(QTextEdit* uiLogTextEditObj,
               const QString& baseLogPath,
               int maxFileSize,
               int maxFileCount)
    : _uiTextEditLog(uiLogTextEditObj)
    , _outputMode(Both)
    , _maxFileSize(maxFileSize)
    , _maxFileCount(qMax(1, maxFileCount))
    , _currentIndex(0)
{
    setBasePath(baseLogPath.isEmpty()
                    ? QCoreApplication::applicationDirPath() + BASE_LOG_FOLDER
                    : baseLogPath);
}

Logger::~Logger() {
    if (_logFile.isOpen()) {
        _logFile.close();
    }
}

void Logger::setUiLogObj(QTextEdit* uiLogTextEditObj) {
    _uiTextEditLog = uiLogTextEditObj;
}

void Logger::setBasePath(const QString& path) {
    if (_logFile.isOpen())
        _logFile.close();

    _basePath = path;

    QDir().mkpath(_basePath);

    _currentIndex = 0;
    openCurrentFile(QIODevice::Append | QIODevice::Truncate | QIODevice::Text);
}

void Logger::setRotationPolicy(int maxSizeBytes, int maxFileCount) {
    _maxFileSize = maxSizeBytes;
    _maxFileCount = qMax(1, maxFileCount);
    if (_currentIndex >= _maxFileCount) {
        _currentIndex = 0;
    }
}

void Logger::setOutputMode(EOutputMode mode) {
    _outputMode = mode;
}

Logger::EOutputMode Logger::outputMode() const {
    return _outputMode;
}

QString Logger::levelToString(ELogLevel level) const {
    switch (level) {
        case Info:
            return "INFO";
        case Warning:
            return "WARNING";
        case Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

QString Logger::fileTimestamp() const {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString Logger::uiTimestamp() const {
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

QString Logger::currentFileName() const {
    QDir dir(_basePath);
    return dir.filePath(QString("%1.%2.log").arg(BASE_LOG_FILE_NAME).arg(_currentIndex));
}

void Logger::openCurrentFile(QIODevice::OpenMode mode) {
    if (_logFile.isOpen()) {
        _logFile.close();
    }
    QString fileName = currentFileName();
    _logFile.setFileName(fileName);
    if (!_logFile.open(mode)) {
        qWarning() << "Logger: невозможно открыть файл для логирования" << fileName;
        return;
    }
    _logStream.setDevice(&_logFile);
}

void Logger::rotateLogs() {
    if (_logFile.isOpen()) {
        _logFile.close();
    }
    _currentIndex = (_currentIndex + 1) % _maxFileCount;
    openCurrentFile(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
}

void Logger::log(const QString& msg, ELogLevel level) {
    if (_logFile.isOpen() && _maxFileSize > 0 && _logFile.size() + msg.size() > _maxFileSize) {
        rotateLogs();
    }

    QString logMsg = QString("[%3] [%1] %2")
                          .arg(levelToString(level), msg);

    if (_outputMode == FileSystemOnly || _outputMode == Both) {
        if (_logStream.device()) {
            _logStream << logMsg.arg(fileTimestamp()) << Qt::endl;
            _logStream.flush();
        }
    }

    if (_outputMode == UIOnly || _outputMode == Both) {
        if (_uiTextEditLog) {
            _uiTextEditLog->append(logMsg.arg(uiTimestamp()));
            _uiTextEditLog->verticalScrollBar()->setValue(
                _uiTextEditLog->verticalScrollBar()->maximum());
        }
    }
}
