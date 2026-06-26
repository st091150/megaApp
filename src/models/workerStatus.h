#ifndef WORKERSTATUS_H
#define WORKERSTATUS_H

#include <QString>

struct WorkerStatus {
    enum class Event {
        None,
        TaskStarted,
        TaskPaused,
        TaskResumed,
        TaskStopped,
        TaskFinished,
        TaskError,
        FileStarted,
        FileProgressUpdate,
        FileFinished,
        FileError
    };

    enum class State {
        Idle,
        Running,
        Paused,
        Finished,
        Stopped,
        Error
    };

    enum class FileProcessStatus {
        Unknown,
        UserStopRequest,
        Ok,
        ProcessFileError,
        Error
    };

    State state = State::Idle;
    Event event = Event::None;

    int totalFiles = 0;
    int processedFiles = 0;

    QString currentFile;
    FileProcessStatus fileProcessStatus = FileProcessStatus::Unknown;
    int fileProgress = 0;
    int taskProgress = 0;

    std::optional<QString> message;
};

#endif // WORKERSTATUS_H
