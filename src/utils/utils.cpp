#include "utils.h"
#include "config.h"
#include "taskModel.h"

#include <QDir>

using namespace WorkerModel;

QByteArray hexStringToByteArray(const QString &hexStr) {
  return QByteArray::fromHex(hexStr.toLatin1());
}

QStringList getFileList(const QString &path, const QString &pattern) {
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

ETaskValidation validateTaskParams(const TaskModel::Task &task) {
  if (task.inputPath.isEmpty())
    return ETaskValidation::InputPathEmpty;

  QDir inputDir(task.inputPath);
  if (!inputDir.exists())
    return ETaskValidation::InputPathNotExists;

  if (task.outputPath.isEmpty())
    return ETaskValidation::OutputPathEmpty;

  QDir outputDir(task.outputPath);
  if (!outputDir.exists() && !outputDir.mkpath("."))
    return ETaskValidation::OutputPathCannotCreate;

  if (task.hexKey.size() != XOR_KEY_BYTES)
    return ETaskValidation::InvalidXorKey;

  return ETaskValidation::Ok;
}

QString errorMessage(ETaskValidation code) {
  switch (code) {
  case ETaskValidation::Ok:
    return QString();
  case ETaskValidation::InputPathEmpty:
    return INPUT_PATH_EMPTY_ERROR_MSG;
  case ETaskValidation::InputPathNotExists:
    return INPUT_FOLDER_DOES_NOT_EXIST_MSG;
  case ETaskValidation::OutputPathEmpty:
    return OUTPUT_EMPTY_PATH_ERROR_MSG;
  case ETaskValidation::OutputPathCannotCreate:
    return OUTPUT_PATH_CREATE_ERROR_MSG;
  case ETaskValidation::InvalidXorKey:
    return invalidHexKeyErrorMsg();
  default:
    return UNKNOWN_TASK_PARAMS_ERROR_MSG;
  }
}

bool isWorkerErrorEvent(EEvent e) {
  switch (e) {
  case EEvent::HexKeySizeError:
  case EEvent::ProcessingError:
  case EEvent::OpenInputFolderError:
  case EEvent::OpenOutputFolderError:
  case EEvent::ReadFileUnknownError:
  case EEvent::FileWriteUnknownError:
  case EEvent::RenameFileError:
  case EEvent::DeleteInputFileError:
  case EEvent::DeleteTempFileError:
    return true;

  default:
    return false;
  }
}

QString workerStateToString(WorkerModel::EState state) {
  switch (state) {
  case WorkerModel::EState::Idle:
    return "Ожидание";
  case WorkerModel::EState::Running:
    return "Выполняется";
  case WorkerModel::EState::Paused:
    return "Пауза";
  case WorkerModel::EState::Stopped:
    return "Остановлено";
  case WorkerModel::EState::Error:
    return "Ошибка";
  default:
    return "Неизвестно";
  }
}

QString workerEventToString(EEvent event) {
  switch (event) {
  case EEvent::TaskStarted:
    return START_TASK_MSG;
  case EEvent::TaskPaused:
    return PAUSED_TASK_MSG;
  case EEvent::TaskResumed:
    return RESUMED_TASK_MSG;
  case EEvent::TaskStopped:
    return STOPPED_TASK_MSG;
  case EEvent::TaskFinished:
    return FINISHED_TASK_MSG;

  case EEvent::FileStarted:
    return STARTED_PROCESS_FILE_MSG;
  case EEvent::FileProgress:
    return FILE_PROGRESS_MSG;
  case EEvent::FileFinished:
    return FILE_FINISHED_MSG;

  case EEvent::UserStopRequest:
    return USER_STOP_REQUEST_MSG;
  case EEvent::UserPauseRequest:
    return USER_PAUSE_REQUEST_MSG;
  case EEvent::UserResumeRequest:
    return USER_RESUME_REQUEST_MSG;

  case EEvent::HexKeySizeError:
    return hexKeySizeErrorMsg();
  case EEvent::ProcessingError:
    return PROCESSING_ERROR_MSG;
  case EEvent::OpenInputFolderError:
    return OPEN_INPUT_FOLDER_ERROR_MSG;
  case EEvent::OpenOutputFolderError:
    return OPEN_OUTPUT_FOLDER_ERROR_MSG;
  case EEvent::ReadFileUnknownError:
    return READ_FILE_UNKNOWN_ERROR_MSG;
  case EEvent::FileWriteUnknownError:
    return FILE_WRITE_UNKNOWN_ERROR_MSG;
  case EEvent::RenameFileError:
    return RENAME_FILE_ERROR_MSG;
  case EEvent::DeleteInputFileError:
    return CANT_DELETE_INPUT_FILE_MSG;
  case EEvent::DeleteTempFileError:
    return TEMP_FILE_DELETE_ERROR_MSG;

  default:
    return UNKNOWN_EVENT_MSG;
  }
}
