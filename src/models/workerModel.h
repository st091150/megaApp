#ifndef WORKERMODEL_H
#define WORKERMODEL_H

#include <QMetaType>
#include <QString>

namespace WorkerModel {

enum class EState { Idle, Running, Paused, Stopped, Error };

enum class EEvent {
  None,

  // Task
  TaskStarted,
  TaskPaused,
  TaskResumed,
  TaskStopped,
  TaskFinished,

  // File
  FileStarted,
  FileProgress,
  FileFinished,

  // User
  UserStopRequest,
  UserPauseRequest,
  UserResumeRequest,

  // Errors
  HexKeySizeError,
  ProcessingError,
  OpenInputFolderError,
  OpenOutputFolderError,
  ReadFileUnknownError,
  FileWriteUnknownError,
  RenameFileError,
  DeleteInputFileError,
  DeleteTempFileError,
};

enum class EUserRequestEvent {
  None,
  Pause,
  Stop,
  // Shutdown
};

struct FileProgress {
  QString currentFile;
  int percent = 0;
};

struct TaskProgress {
  int totalFiles = 0;
  int processedFiles = 0;
  int percent = 0;
};

struct Status {
  EEvent event = EEvent::None;

  FileProgress fileStatusInfo;
  TaskProgress taskStatusInfo;
};

} // namespace WorkerModel

Q_DECLARE_METATYPE(WorkerModel::Status)

#endif // WORKERMODEL_H
