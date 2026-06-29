#include "worker.h"
#include "config.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QThread>


Worker::Worker(QObject *parent) : QObject(parent) {}

bool Worker::isPaused() {
    QMutexLocker lock(&_mutex);
    return _state == State::Paused;
}

bool Worker::isBusy() {
    QMutexLocker lock(&_mutex);
    return _state == State::Running || _state == State::Paused;
}

void Worker::initNewTask(const Task &newTask) {
    Status newStatus;

    newStatus.event = Event::None;
    newStatus.taskStatusInfo.totalFiles = newTask.files.size();

    _currentTaskStatus = newStatus;
    _state = State::Running;

    QMutexLocker lock(&_mutex);
    _userRequest = EUserRequestEvent::None;
}

void Worker::emitStatus(Event event) {
    if (_currentTaskStatus) {
        _currentTaskStatus->event = event;
        emit statusUpdate(*_currentTaskStatus);
    }
}

void Worker::run(const Task& task) {
  initNewTask(task);

  if (task.hexKey.size() != ALGORITHM_KEY_SIZE) {
    changeState(State::Idle, Event::HexKeySizeError);
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
  memcpy(&keyWord, task.hexKey.constData(), ALGORITHM_KEY_SIZE);
  for (const QString &file : std::as_const(task.files)) {
      _currentTaskStatus->fileStatusInfo.currentFile = file;
      if (handleUserRequest()) {
          changeState(State::Idle, EEvent::UserStopRequest);
          return;
      }
    emitStatus(Event::FileStarted);

    QString inputFilePath = QDir(task.inputPath).filePath(file);
    QString outputFilePath = buildOutputFilePath(task.outputPath, file, task.duplicateAction);

    Event fileProcessingResult = processFile(inputFilePath,
                                        outputFilePath,
                                        task.deleteSourceFlag,
                                        keyWord,
                                        totalBytes,
                                        processedBytes);

    if (fileProcessingResult == EEvent::UserStopRequest) {
        changeState(State::Idle, EEvent::UserStopRequest);
        return;
    }
    if (fileProcessingResult == Event::FileFinished)
        _currentTaskStatus->taskStatusInfo.processedFiles++;

    emitStatus(fileProcessingResult);
  }

  changeState(State::Idle, Event::TaskFinished);
}

void Worker::start(const Worker::Task& newTask) {
    if (!isBusy()){
        run(newTask);
    }
}

bool Worker::handleUserRequest() {
    bool isWasPaused = false;
  QMutexLocker lock(&_mutex);
  while (_userRequest == EUserRequestEvent::Pause) {
      changeState(State::Paused, EEvent::UserPauseRequest);
      isWasPaused = true;
    _pauseCond.wait(&_mutex);
  }
  if (_userRequest == EUserRequestEvent::Stop) {
      return true;
  } else if (isWasPaused) {
      changeState(State::Running, EEvent::UserResumeRequest);
      return false;
  }
  return false;
}

void Worker::wordXor(char *data, qint64 size, uint64_t key) {
    if (!data || size <= 0)
        return;

    const uint8_t *k = reinterpret_cast<const uint8_t *>(&key);
    qint64 aligned = size - (size % sizeof(uint64_t));

    for (qint64 i = 0; i < aligned; i += sizeof(uint64_t)) {
        uint64_t word;
        memcpy(&word, data + i, sizeof(word));
        word ^= key;
        memcpy(data + i, &word, sizeof(word));
    }
    for (qint64 j = 0; aligned < size; ++j) {
        data[aligned++] ^= k[j % 8];
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

QString Worker::buildOutputFilePath(const QString &outputFilePath,
                                    const QString &file,
                                    const EDuplicateAction duplicationFlag) const
{
  QString outputPath = QDir(outputFilePath).filePath(file);

  if (duplicationFlag == EDuplicateAction::Overwrite) {
    return outputPath;
  }

  QFileInfo fi(outputPath);

  const QString baseName = fi.completeBaseName();
  const QString suffix = fi.suffix();

  int counter = 1;
  while (QFile::exists(outputPath)) {
    QString newName = QString("%1_%2").arg(baseName).arg(counter++);
    if (!suffix.isEmpty()) {
      newName += "." + suffix;
    }
    outputPath = QDir(outputFilePath).filePath(newName);
  }

  return outputPath;
}

EEvent Worker::processFile(const QString& inputFilePath,
                          const QString& outputFilePath,
                          bool deleteSourceFlag,
                          uint64_t keyWord,
                          qint64 totalBytes,
                          qint64 &processedBytes) {

  const bool sameFile = (inputFilePath == outputFilePath);

  const QString finalOutputFilePath =
      sameFile ? makeUniqueTempPath(outputFilePath) : outputFilePath;

  QFile input(inputFilePath);
  QFile output(finalOutputFilePath);

  if (!input.open(QIODevice::ReadOnly)) {
    return Event::OpenInputFolderError;
  }

  if (!output.open(QIODevice::WriteOnly)) {
    return Event::OpenOutputFolderError;
  }

  const auto cleanUp = [&output, &input, finalOutputFilePath]() {
    output.flush();
    output.close();
    input.close();
    QFile::remove(finalOutputFilePath);
  };

  QByteArray buffer(CHUNK_SIZE, Qt::Uninitialized);

  qint64 processed = 0;
  int lastProgress = -1;
  const qint64 totalSize = input.size();

  while (!input.atEnd()) {

    if (handleUserRequest())
      return Event::UserStopRequest;

    const qint64 readBytes = input.read(buffer.data(), CHUNK_SIZE);

    if (readBytes < 0) {
      processedBytes += (totalSize - processed);
      cleanUp();
      return Event::ReadFileUnknownError;
    } else if (readBytes == 0)
      break;

    wordXor(buffer.data(), readBytes, keyWord);

    qint64 totalWritten = 0;
    while (totalWritten < readBytes) {
      const qint64 w = output.write(buffer.constData() + totalWritten,
                                    readBytes - totalWritten);

      if (w <= 0) {
        cleanUp();
        if (handleUserRequest()) {
            return Event::UserStopRequest;
        } else {
            return Event::FileWriteUnknownError;
        }
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

      _currentTaskStatus->fileStatusInfo.percent = fileProgress;
      _currentTaskStatus->taskStatusInfo.percent = taskProgress;
    }

    if (handleUserRequest()) {
      cleanUp();
      return Event::UserStopRequest;
    }
    emitStatus(Event::FileProgress);
  }

  if (!output.flush()) {
    return Event::FileWriteUnknownError;
  }

  input.close();
  output.close();

  if (sameFile) {
    if (!QFile::remove(outputFilePath) && QFile::exists(outputFilePath)) {
      return Event::DeleteTempFileError;
    }

    if (!QFile::rename(finalOutputFilePath, outputFilePath)) {
      return Event::RenameFileError;
    }
  }

  if (deleteSourceFlag) {
    if (!QFile::remove(inputFilePath)) {
      return Event::DeleteInputFileError;
    }
  }

  return Event::FileFinished;
}

void Worker::changeState(State newState, Event trigger) {
    _state = newState;
    if (_currentTaskStatus) {
        _currentTaskStatus->event = trigger;
        emit statusUpdate(*_currentTaskStatus);
    }
}

void Worker::pause() {
QMutexLocker lock(&_mutex);
    _userRequest = EUserRequestEvent::Pause;
}

void Worker::resume() {
    QMutexLocker lock(&_mutex);
    _userRequest = EUserRequestEvent::None;
    _pauseCond.wakeAll();
}

void Worker::shutdown() {
  QMutexLocker lock(&_mutex);
    _userRequest = EUserRequestEvent::Stop;
  _pauseCond.wakeAll();
}

void Worker::stop() {
  QMutexLocker lock(&_mutex);
    _userRequest = EUserRequestEvent::Stop;
  _pauseCond.wakeAll();
}
