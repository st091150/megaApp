#include "worker.h"
#include "config.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QThread>


Worker::Worker(QObject *parent) : QObject(parent) {
  _status.state = State::Idle;
}

void Worker::initNewTask(const TaskParams &newTask) {
  QMutexLocker lock(&_mutex);
  _stopped = false;
  _paused = false;
  _status = WorkerStatus();
  _status.event = Event::TaskStarted;
  _status.totalFiles = newTask.files.size();
  _status.message = "Запуск выполнения";
  _status.state = State::Running;
  _status.taskProgress = 0;
}

void Worker::emitStatus(Event event) {
  WorkerStatus copy;
  {
    QMutexLocker lock(&_mutex);
    if (event != Event::None)
      _status.event = event;
    copy = _status;
  }
  emit statusChanged(copy);
}

void Worker::run(TaskParams task) {
  initNewTask(task);

  if (task.hexKey.size() != ALGORITHM_HEX_KEY_SIZE) {
    finish(QString("Неверный размер ключа (%1, ожидаемый размер %2)")
               .arg(task.hexKey.size(), ALGORITHM_HEX_KEY_SIZE),
           State::Error, Event::TaskError);
    return;
  }

  qint64 totalBytes = 0;
  QDir inputDir(task.inputPath);
  for (const QString &file : std::as_const(task.files)) {
    QFileInfo info(inputDir.filePath(file));
    if (info.exists() && info.isFile())
      totalBytes += info.size();
  }
  qint64 processedBytes = 0;

  uint64_t keyWord;
  memcpy(&keyWord, task.hexKey.constData(), ALGORITHM_HEX_KEY_SIZE);
  for (const QString &file : std::as_const(task.files)) {
    setCurrentFile(file);
    setStatusMessage(QString("Начало обработки %1").arg(file));

    if (isUserRequest()) {
      finish(USER_STOP_REQUEST, State::Stopped, Event::TaskStopped);
      return;
    }
    emitStatus(Event::FileStarted);

    FileProcessStatus result =
        processFile(task, file, keyWord, totalBytes, processedBytes);

    switch (result) {
    case FileProcessStatus::Ok: {
      setStatusMessage(QString("%1 файл обработан.").arg(file));
      addProcessedFile();
      if (isUserRequest()) {
        finish(USER_STOP_REQUEST, State::Stopped, Event::TaskStopped);
        return;
      }
      emitStatus(Event::FileFinished);
      break;
    }
    case FileProcessStatus::UserStopRequest: {
      finish(USER_STOP_REQUEST, State::Stopped, Event::TaskStopped);
      return;
    }
    default: {
      if (isUserRequest()) {
        finish(USER_STOP_REQUEST, State::Stopped, Event::TaskStopped);
        return;
      }
      emitStatus(Event::FileError);
    }
    }
  }

  finish(TASK_FINISHED, State::Finished, Event::TaskFinished);
}

void Worker::start(TaskParams newTask) {
  {
    QMutexLocker lock(&_mutex);
    if (_status.state == State::Running || _status.state == State::Paused) {
      return;
    }
  }
  run(std::move(newTask));
}

void Worker::setStatusState(State newState) {
  QMutexLocker lock(&_mutex);
  _status.state = newState;
}

void Worker::setEvent(Event event) {
  QMutexLocker lock(&_mutex);
  _status.event = event;
}

void Worker::setStatusMessage(const QString &message) {
  QMutexLocker lock(&_mutex);
  _status.message = message;
}

void Worker::setCurrentFile(const QString &fileName) {
  QMutexLocker lock(&_mutex);
  _status.currentFile = fileName;
  _status.fileProgress = 0;
}

void Worker::setFileProgress(int progress) {
  QMutexLocker lock(&_mutex);
  _status.fileProgress = progress;
}

void Worker::addProcessedFile() {
  QMutexLocker lock(&_mutex);
  _status.processedFiles++;
}

void Worker::setTaskProgress(int progressPercent) {
  if (progressPercent < 0 || progressPercent > 100)
    return;
  QMutexLocker lock(&_mutex);
  _status.fileProgress = progressPercent;
}

bool Worker::isPaused() { return _paused; }

bool Worker::isBusy() {
  QMutexLocker lock(&_mutex);
  return _status.state == State::Running || _status.state == State::Paused;
}

bool Worker::isUserRequest() {
  QMutexLocker lock(&_mutex);
  while (_paused && !_stopped) {
    _pauseCond.wait(&_mutex);
  }
  return _stopped;
}

void Worker::wordXor(char *data, qint64 size, uint64_t key) {
  for (qint64 i = 0; i + sizeof(uint64_t) <= size; i += sizeof(uint64_t)) {
    uint64_t word;
    memcpy(&word, data + i, sizeof(word));
    word ^= key;
    memcpy(data + i, &word, sizeof(word));
  }

  const uint8_t *k = reinterpret_cast<const uint8_t *>(&key);

  uint8_t ki = 0;
  for (qint64 i = (size / 8) * 8; i < size; ++i) {
    data[i] ^= k[ki++];
    ki %= 8;
  }
}

QString Worker::makeUniqueTempPath(const QString &finalPath) const {
  QFileInfo fi(finalPath);

  QString base = fi.completeBaseName();
  QString suffix = fi.suffix().isEmpty() ? "" : "." + fi.suffix();

  QByteArray seed = QByteArray::number(QDateTime::currentMSecsSinceEpoch());

  while (true) {
    QByteArray hash =
        QCryptographicHash::hash(seed, QCryptographicHash::Md5).toHex().left(8);

    QString candidate =
        fi.dir().filePath(base + ".tmp." + QString::fromLatin1(hash) + suffix);
    if (!QFile::exists(candidate)) {
      return candidate;
    }
    seed.append(hash);
  }
}

QString Worker::buildOutputFilePath(const TaskParams &task,
                                    const QString &file) const {
  QString outputFilePath = QDir(task.outputPath).filePath(file);

  if (task.duplicateAction == TaskParams::DuplicateAction::Overwrite) {
    return outputFilePath;
  }

  QFileInfo fi(outputFilePath);

  const QString baseName = fi.completeBaseName();
  const QString suffix = fi.suffix();

  int counter = 1;
  while (QFile::exists(outputFilePath)) {
    QString newName = QString("%1_%2").arg(baseName).arg(counter++);
    if (!suffix.isEmpty()) {
      newName += "." + suffix;
    }
    outputFilePath = QDir(task.outputPath).filePath(newName);
  }

  return outputFilePath;
}

WorkerStatus::FileProcessStatus Worker::processFile(const TaskParams &task,
                                                    const QString &file,
                                                    uint64_t keyWord,
                                                    qint64 totalBytes,
                                                    qint64 &processedBytes) {
  const QString inputFilePath = QDir(task.inputPath).filePath(file);
  const QString finalOutputPath = buildOutputFilePath(task, file);

  const bool sameFile = (inputFilePath == finalOutputPath);

  const QString outputFilePath =
      sameFile ? makeUniqueTempPath(finalOutputPath) : finalOutputPath;

  QFile input(inputFilePath);
  QFile output(outputFilePath);

  if (!input.open(QIODevice::ReadOnly)) {
    setStatusMessage(OPEN_INPUT_FOLDER_ERROR);
    return FileProcessStatus::Error;
  }

  if (!output.open(QIODevice::WriteOnly)) {
    setStatusMessage(OPEN_OUTPUT_FOLDER_ERROR);
    return FileProcessStatus::Error;
  }

  const auto cleanUp = [&output, &input, outputFilePath]() {
    output.flush();
    output.close();
    input.close();
    QFile::remove(outputFilePath);
  };

  QByteArray buffer(CHUNK_SIZE, Qt::Uninitialized);

  qint64 processed = 0;
  int lastProgress = -1;
  const qint64 totalSize = input.size();

  while (!input.atEnd()) {

    if (isUserRequest())
      return FileProcessStatus::UserStopRequest;

    const qint64 readBytes = input.read(buffer.data(), CHUNK_SIZE);

    if (readBytes < 0) {
      processedBytes += (totalSize - processed);
      setTaskProgress(
          (totalBytes == 0)
              ? 100
              : static_cast<int>((processedBytes * 100) / totalBytes));
      setStatusMessage(READ_FILE_UNKNOWN_ERROR);
      cleanUp();
      return FileProcessStatus::ProcessFileError;
    } else if (readBytes == 0)
      break;

    wordXor(buffer.data(), readBytes, keyWord);

    qint64 totalWritten = 0;
    while (totalWritten < readBytes) {
      const qint64 w = output.write(buffer.constData() + totalWritten,
                                    readBytes - totalWritten);

      if (w <= 0) {
        if (isUserRequest()) {
          cleanUp();
          return FileProcessStatus::UserStopRequest;
        }
        setStatusMessage(FILE_WRITE_UNKNOWN_ERROR);
        cleanUp();
        return FileProcessStatus::ProcessFileError;
      }

      totalWritten += w;
    }

    processed += readBytes;
    processedBytes += readBytes;

    int fileProgress = (totalSize == 0)
                           ? 100
                           : static_cast<int>((processed * 100) / totalSize);
    int taskProgress =
        (totalBytes == 0)
            ? 100
            : static_cast<int>((processedBytes * 100) / totalBytes);

    if (fileProgress != lastProgress) {
      lastProgress = fileProgress;

      QMutexLocker lock(&_mutex);
      _status.fileProgress = fileProgress;
      _status.taskProgress = taskProgress;
    }

    if (isUserRequest()) {
      cleanUp();
      return FileProcessStatus::UserStopRequest;
    }

    emitStatus(Event::FileProgressUpdate);
  }

  if (!output.flush()) {
    setStatusMessage(FILE_WRITE_UNKNOWN_ERROR);
    return FileProcessStatus::ProcessFileError;
  }

  input.close();
  output.close();

  if (sameFile) {
    if (!QFile::remove(finalOutputPath) && QFile::exists(finalOutputPath)) {
      setStatusMessage(OLD_FILE_DELETE_ERROR);
      return FileProcessStatus::Error;
    }

    if (!QFile::rename(outputFilePath, finalOutputPath)) {
      setStatusMessage(RENAME_FILE_ERROR);
      return FileProcessStatus::Error;
    }
  }

  if (task.deleteSourceFlag) {
    if (!QFile::remove(inputFilePath)) {
      setStatusMessage(QString(CANT_DELETE_INPUT_FILE) + " " + file);
      return FileProcessStatus::Error;
    }
  }

  return FileProcessStatus::Ok;
}

void Worker::finish(const QString &message, State finalState, Event event) {
  WorkerStatus copy;
  {
    QMutexLocker lock(&_mutex);
    _status.message = message;
    _status.state = finalState;
    _status.event = event;
    copy = _status;
  }
  emit statusChanged(copy);
}

void Worker::pause() {
  WorkerStatus copy;
  {
    QMutexLocker lock(&_mutex);
    if (_paused || _status.state != State::Running)
      return;
    _paused = true;
    _status.state = State::Paused;
    _status.event = Event::TaskPaused;
    _status.message = "Выполнение приостановлено";
    copy = _status;
  }
  emit statusChanged(copy);
}

void Worker::resume() {
  WorkerStatus copy;
  {
    QMutexLocker lock(&_mutex);
    if (_status.state != State::Paused)
      return;

    _paused = false;

    _status.state = State::Running;
    _status.event = Event::TaskResumed;
    _status.message = "Возобновление работы";
    copy = _status;
    _pauseCond.wakeAll();
  }
  emit statusChanged(copy);
}

void Worker::shutdown() {
  QMutexLocker lock(&_mutex);

  _stopped = true;
  _paused = false;

  _pauseCond.wakeAll();
}

void Worker::stop() {
  QMutexLocker lock(&_mutex);
  if (_stopped && _status.state != State::Running &&
      _status.state != State::Paused)
    return;

  _stopped = true;

  _status.state = State::Stopped;
  _status.message = "Остановка работы";
  _pauseCond.wakeAll();
}
