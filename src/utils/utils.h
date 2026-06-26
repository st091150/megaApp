#ifndef UTILS_H
#define UTILS_H

#include "workerStatus.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

QByteArray hexStringToByteArray(const QString &hexStr);

QStringList getFileList(const QString &path, const QString &pattern);

QString workerStateToString(WorkerStatus::State status);

#endif // UTILS_H
