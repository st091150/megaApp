#ifndef ERRORMSG_H
#define ERRORMSG_H

#include <worker.h>

#include <QString>

enum class ParamsValidationResult {
  Ok,
  InputPathEmpty,
  InputPathNotExists,
  OutputPathEmpty,
  OutputPathCannotCreate,
  InvalidXorKey
};

enum class WorkerError {
  None,
  BadHexKeySize,

};

QString errorMessage(ParamsValidationResult code);

QString workerStateToString(WorkerStatus::State state);

QString workerEventToString(WorkerStatus::Event event);

#endif // ERRORMSG_H
