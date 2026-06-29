#ifndef WORKER_H
#define WORKER_H

#include "taskModel.h"
#include "workerModel.h"

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

using namespace WorkerModel;
using namespace TaskModel;

class Worker : public QObject {
  Q_OBJECT

public:
friend class TestWorkerXor;

private:
    using Task = TaskModel::Task;

    using Event = WorkerModel::EEvent;
    using State = WorkerModel::EState;
    using Status = WorkerModel::Status;

public:
  explicit Worker(QObject *parent = nullptr);
  ~Worker() = default;

public:
  bool isBusy();
  bool isPaused();

public slots:
  void start(const Worker::Task& newTask);
  void pause();
  void resume();
  void stop();
  void shutdown();

signals:
  void statusUpdate(const Worker::Status &status);

private:
  void run(const Task& task);
  void initNewTask(const Task &params);

  void changeState(State newState, Event trigger);

  void emitStatus(Event event);

  bool handleUserRequest();

  QString makeUniqueTempPath(const QString &finalPath) const;

  QString buildOutputFilePath(const QString &outputFilePath,
                              const QString &file,
                              EDuplicateAction duplicationFlag) const;

  Event processFile(const QString& inputFilePath,
                    const QString& outputFilePath,
                    bool deleteSourceFlag,
                    uint64_t keyWord,
                    qint64 totalBytes,
                    qint64 &processedBytes);

  void wordXor(char *data, qint64 size, uint64_t keyWord);



private:
  std::optional<Status> _currentTaskStatus;

  QMutex _mutex;
  State _state = State::Idle;
  EUserRequestEvent _userRequest;
  QWaitCondition _pauseCond;
};

#endif // WORKER_H
