#ifndef UTILS_H
#define UTILS_H

#include "taskModel.h"
#include "workerModel.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

enum class ETaskValidation {
  Ok,
  InputPathEmpty,
  InputPathNotExists,
  OutputPathEmpty,
  OutputPathCannotCreate,
  InvalidXorKey
};

QByteArray hexStringToByteArray(const QString &hexStr);

QStringList getFileList(const QString &path, const QString &pattern);

ETaskValidation validateTaskParams(const TaskModel::Task &task);
QString errorMessage(ETaskValidation code);

bool isWorkerErrorEvent(WorkerModel::EEvent event);
QString workerStateToString(WorkerModel::EState state);
QString workerEventToString(WorkerModel::EEvent event);

#endif // UTILS_H
