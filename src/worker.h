#ifndef WORKER_H
#define WORKER_H

#include "taskParams.h"
#include "workerStatus.h"

#include <QMutex>
#include <QObject>
#include <QWaitCondition>


class Worker : public QObject {
  Q_OBJECT

public:
friend class TestWorkerXor;

private:
  using State = WorkerStatus::State;
  using Event = WorkerStatus::Event;
  using FileProcessStatus = WorkerStatus::FileProcessStatus;

public:
  explicit Worker(QObject *parent = nullptr);
  ~Worker() = default;

public:
  bool isBusy();
  bool isPaused();

public slots:
  void start(TaskParams params);
  void pause();
  void resume();
  void stop();
  void shutdown();

signals:
  void statusChanged(const WorkerStatus &status);

private:
  void run(TaskParams task);
  void initNewTask(const TaskParams &params);
  void emitStatus(Event event = Event::None);

  void setEvent(Event event);
  void setStatusState(State newState);
  void setStatusMessage(const QString &message);
  void setCurrentFile(const QString &fileName);
  void setFileProgress(int progress);
  void setTaskProgress(int progressPercent);

  void addProcessedFile();

  bool isUserRequest();

  QString makeUniqueTempPath(const QString &finalPath) const;

  QString buildOutputFilePath(const TaskParams &task,
                              const QString &file) const;

  FileProcessStatus processFile(const TaskParams &task, const QString &file,
                                uint64_t keyWord, qint64 totalBytes,
                                qint64 &processedBytes);

  void wordXor(char *data, qint64 size, uint64_t keyWord);

  void finish(const QString &message, State finalState, Event event);

private:
  WorkerStatus _status;

  std::atomic<bool> _stopped = false;
  std::atomic<bool> _paused = false;

  QMutex _mutex;
  QWaitCondition _pauseCond;
};

#endif // WORKER_H
