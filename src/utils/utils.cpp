#include "utils.h"

#include <QDir>

QByteArray hexStringToByteArray(const QString& hexStr) {
    return QByteArray::fromHex(hexStr.toLatin1());
}

QStringList getFileList(const QString& path, const QString& pattern) {
    QDir dir(path);
    if (!dir.exists()) {
        return QStringList();
    }

    QString trimmedPattern = pattern.trimmed();
    if (trimmedPattern.isEmpty()) {
        return dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    }

    QStringList filters = trimmedPattern.split(',', Qt::SkipEmptyParts);
    for (QString &f : filters) {
        f = f.trimmed();
        f = "*" + f + "*";
    }
    filters.removeAll(QString());
    filters.removeDuplicates();

    if (filters.isEmpty()) {
        return dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    }

    return dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
}

QString workerEventToString(WorkerStatus::State state){
    switch (state) {
    case WorkerStatus::State::Idle:
        return "Ожидание";
    case WorkerStatus::State::Running:
        return "Выполняется";
    case WorkerStatus::State::Paused:
        return "Пауза";
    case WorkerStatus::State::Stopped:
        return "Остановлено";
    case WorkerStatus::State::Finished:
        return "Завершено";
    case WorkerStatus::State::Error:
        return "Ошибка";
    default:
        return "Неизвестно";
    }
}
