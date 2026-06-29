#include "mainwindow.h"
#include "config.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MegaApp), logger(new Logger) {
  ui->setupUi(this);
  init();
}

void MainWindow::initStatusBars() {
  ui->processingState->setText(QString(PROCESSING_LABEL_PLACEHOLDER));
  ui->processingStatusBar->setValue(0);

  ui->labelCurrentFile->setText(
      QString(FILE_PROCESSING_LABEL_TEMPLATE).arg(""));
  ui->currentFileStatusBar->setValue(0);

  ui->intervalTimerBox->setVisible(ui->TimerRunModeBtn->isChecked());
  ui->pauseBtn->setText("Приостановить");
}

void MainWindow::updateStatusBars(int processedFileCount, int totalFiles,
                                  int processingPercentage,
                                  const QString &processingFileName,
                                  int fileProcessingPercentage) {
  ui->processingState->setText(QString(PROCESSING_LABEL_TEMPLATE)
                                   .arg(processedFileCount)
                                   .arg(totalFiles));
  ui->processingStatusBar->setValue(processingPercentage);

  ui->labelCurrentFile->setText(
      QString(FILE_PROCESSING_LABEL_TEMPLATE).arg(processingFileName));
  ui->currentFileStatusBar->setValue(fileProcessingPercentage);
}

void MainWindow::init() {
  logger->setUiLogObj(ui->uiLog);

  QIcon folderIcon = style()->standardIcon(QStyle::SP_DirOpenIcon);
  ui->btnBrowseInput->setIcon(folderIcon);
  ui->btnBrowseOutput->setIcon(folderIcon);

  ui->timerIntervalWidget->setVisible(false);
  ui->intervalTimerBox->setVisible(false);

  ui->timerIntervalSpinBox->setMinimum(MIN_TIMER_INTERVAL);

  ui->xorKeyLineEdit->setPlaceholderText(hexKeyPlaceholder());
  QRegularExpression hexRegex(hexPattern());
  QRegularExpressionValidator *validator =
      new QRegularExpressionValidator(hexRegex, this);
  ui->xorKeyLineEdit->setValidator(validator);

  initStatusBars();

  connect(ui->TimerRunModeBtn, &QCheckBox::toggled, this, [this]() {
    if (!_prevTask || (_prevTask->runMode != ERunMode::Timer || !_isPaused) &&
                          !_tickTimer.isActive()) {
      ui->intervalTimerBox->setVisible(ui->TimerRunModeBtn->isChecked());
    }
  });

  connect(ui->btnBrowseInput, &QPushButton::clicked, this,
          &MainWindow::browseInputDirectory);
  connect(ui->btnBrowseOutput, &QPushButton::clicked, this,
          &MainWindow::browseOutputDirectory);

  connect(ui->startBtn, &QPushButton::clicked, this, &MainWindow::onStart);
  connect(ui->pauseBtn, &QPushButton::clicked, this,
          &MainWindow::onPauseResume);
  connect(ui->stopBtn, &QPushButton::clicked, this, &MainWindow::onStop);

  connect(&_tickTimer, &QTimer::timeout, this, &MainWindow::onTick);
  _tickTimer.setInterval(TIMER_DISPLAY_UPDATE_TIME);

  connect(ui->textCleanBtn, &QPushButton::clicked, this,
          [this]() { ui->uiLog->setText(""); });

  _remainingMs = 0;
  _isPaused = false;

  _thread = new QThread;
  _worker = new Worker();
  _worker->moveToThread(_thread);

  connect(_thread, &QThread::finished, _worker, &QObject::deleteLater);
  connect(_thread, &QThread::finished, _thread, &QObject::deleteLater);

  connect(_worker, &Worker::statusUpdate, this, &MainWindow::parseWorkerStatus);

  _thread->start();
}

MainWindow::~MainWindow() {
  if (_worker) {
    _worker->stop();
  }

  if (_thread) {
    _thread->quit();
    _thread->wait();
  }

  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (_worker) {
    logMessage("Завершение", Logger::Info);
    _worker->stop();
  }

  if (_thread) {
    _thread->quit();

    if (!_thread->wait(5000)) {
      logMessage("Поток не завершился за 5 сек", Logger::Warning);
    }
  }

  event->accept();
}

TaskModel::Task MainWindow::createTask() const {
  QStringList files = getFileList(ui->inputDirLineEdit->text(),
                                  ui->inputFileMaskLineEdit->text());
  QByteArray hexKey = hexStringToByteArray(ui->xorKeyLineEdit->text());

  Task Task{.inputPath = ui->inputDirLineEdit->text(),
            .outputPath = ui->outputDirLineEdit->text(),
            .fileMask = ui->inputFileMaskLineEdit->text(),
            .files = files,
            .hexKey = hexKey,
            .deleteSourceFlag = ui->deleteSourceCheckBox->isChecked(),
            .timerSecTime = ui->timerIntervalSpinBox->value(),
            .duplicateAction = ui->overwriteBtn->isChecked()
                                   ? EDuplicateAction::Overwrite
                                   : EDuplicateAction::AddCounter,
            .runMode = ui->OnceRunMode->isChecked() ? ERunMode::Once
                                                    : ERunMode::Timer};

  return Task;
}

bool MainWindow::validateTimerSettings() const {
  return ui->timerIntervalSpinBox->value() > 0;
}

void MainWindow::handleValidationError(ETaskValidation error) {
  QString msg = errorMessage(error);
  logMessage(msg, LogLevel::Error);
}

void MainWindow::runTask(bool fromTimer) {
  if (isBusy()) {
    return;
  }
  if (!_prevTask) {
    logMessage("Ошибка: задача не инициализирована.", Logger::Error);
    return;
  }

  _prevTask->files = getFileList(_prevTask->inputPath, _prevTask->fileMask);
  if (_prevTask->files.isEmpty()) {
    warningLogMessage("Файлы по маске не найдены.");
    return;
  }

  if (fromTimer) {
    logMessage("=== Запуск (по таймеру) ===", Logger::Info);
  } else {
    logMessage("=== Запуск (разовый) ===", Logger::Info);
  }

  logTask(*_prevTask);
  startProcessing(*_prevTask);
}

void MainWindow::startProcessing(const Task &task) {
  if (!_worker)
    return;
  QMetaObject::invokeMethod(_worker, "start", Qt::QueuedConnection,
                            Q_ARG(Task, task));
}

void MainWindow::logMessage(const QString &msg, LogLevel logLevel) {
  if (!logger)
    return;

  switch (logLevel) {
  case LogLevel::Info:
    logger->info(msg);
    break;
  case Logger::Warning:
    logger->warning(msg);
    break;
  case Logger::Error:
    logger->error(msg);
    break;
  }
}

void MainWindow::logMessage(const QString &msg, LogLevel logLevel,
                            OutputMode mode) {
  if (!logger)
    return;

  OutputMode oldMode = logger->outputMode();
  bool needToSaveMode = !(oldMode == mode);

  logger->setOutputMode(mode);
  logMessage(msg, logLevel);

  if (needToSaveMode) {
    logger->setOutputMode(oldMode);
  }
}

void MainWindow::browseInputDirectory() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Выберите папку для поиска файлов"),
      ui->inputDirLineEdit->text(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dir.isEmpty())
    ui->inputDirLineEdit->setText(dir);
}

void MainWindow::browseOutputDirectory() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Выберите папку для сохранения результатов"),
      ui->outputDirLineEdit->text(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dir.isEmpty())
    ui->outputDirLineEdit->setText(dir);
}

void MainWindow::logTask(const Task &params) {
  OutputMode oldMode = logger->outputMode();

  logger->setOutputMode(OutputMode::FileSystemOnly);

  infoLogMessage(QString("Входная папка: %1").arg(params.inputPath));
  infoLogMessage(QString("Выходная папка: %1").arg(params.outputPath));

  infoLogMessage(
      QString("Маска: %1")
          .arg(params.fileMask.isEmpty() ? "(все файлы)" : params.fileMask));
  infoLogMessage(QString("Удалять исходные файлы: %1")
                     .arg(params.deleteSourceFlag ? "Да" : "Нет"));

  QString dupAction = (params.duplicateAction == EDuplicateAction::Overwrite)
                          ? "Перезапись"
                          : "Добавить счетчик к имени";
  infoLogMessage(QString("Действие при повторе: %1").arg(dupAction));
  infoLogMessage(
      QString("Ключ XOR: %1").arg(QString::fromLatin1(params.hexKey.toHex())));
  infoLogMessage(
      QString("Режим: %1")
          .arg(params.runMode == ERunMode::Once ? "Разовый" : "По таймеру"));
  if (params.runMode == ERunMode::Timer) {
    infoLogMessage(QString("Интервал таймера: %1 сек")
                       .arg(ui->timerIntervalSpinBox->value()));
  }

  logger->setOutputMode(OutputMode::Both);

  QStringList fileNames = params.files;
  qint64 totalSize = 0;
  QDir inputDir(params.inputPath);
  for (const QString &name : std::as_const(fileNames)) {
    QFileInfo info(inputDir.absoluteFilePath(name));
    if (info.exists() && info.isFile()) {
      totalSize += info.size();
    }
  }

  infoLogMessage(QString("Найдено файлов: %1").arg(fileNames.size()));
  if (totalSize > 0) {
    double sizeMB = totalSize / (1024.0 * 1024.0);
    infoLogMessage(
        QString("Общий размер файлов: %1 МБ").arg(sizeMB, 0, 'f', 2));
  }

  QString logFilesName = "Список файлов для обработки:";
  for (const QString &name : std::as_const(fileNames)) {
    logFilesName += QString("  %1").arg(name);
  }
  logMessage(logFilesName, Logger::Info, Logger::FileSystemOnly);

  logger->setOutputMode(oldMode);
}

void MainWindow::parseWorkerStatus(const Status &s) {
  if (_worker) {
    _isPaused = _worker->isPaused();
  }

  if (_isPaused) {
    ui->pauseBtn->setText("Возобновить");
  } else {
    ui->pauseBtn->setText("Приостановить");
  }

  if (s.event == EEvent::UserStopRequest) {
    initStatusBars();
  } else {
    updateStatusBars(s.taskStatusInfo.processedFiles,
                     s.taskStatusInfo.totalFiles, s.taskStatusInfo.percent,
                     s.fileStatusInfo.currentFile, s.fileStatusInfo.percent);
  }

  if (s.event == EEvent::FileProgress) {
    return;
  }
  logger->setOutputMode(Logger::Both);

  logMessage("=== [WORKER] ===", Logger::Info, OutputMode::FileSystemOnly);

  if (isWorkerErrorEvent(s.event)) {
    logMessage(QString("Событие: ") + workerEventToString(s.event),
               Logger::Error);
  } else {
    logMessage(QString("Событие: ") + workerEventToString(s.event),
               Logger::Info);
  }

  infoLogMessage(
      QString("Текущий файл обработки: %1").arg(s.fileStatusInfo.currentFile));
  infoLogMessage(
      QString("Прогресс обработки файла: %1%").arg(s.fileStatusInfo.percent));
  infoLogMessage(QString("Обработано файлов: %1/%2")
                     .arg(s.taskStatusInfo.processedFiles)
                     .arg(s.taskStatusInfo.totalFiles));

  // const QString fileStatus = QString("Статус обработки файла:
  // %1").arg(workerStateToString(s.state)); if (s.fileProcessStatus ==
  // WorkerStatus::FileProcessStatus::Error){
  //     logMessage(fileStatus, Logger::Error, Logger::Both);
  // } else {
  //     infoLogMessage(fileStatus);
  // }
}

void MainWindow::onTick() {
  if (!_prevTask)
    return;

  if (_prevTask->runMode != ERunMode::Timer)
    return;

  qint64 ms = QDateTime::currentDateTime().msecsTo(_timerEndTime);

  if (ms <= 0) {
    ui->intervalTimer->setTime(QTime(0, 0, 0));

    runTask(true);

    int intervalSec;
    if (_prevTask->timerSecTime == 0)
      intervalSec = ui->timerIntervalSpinBox->value();
    else {
      intervalSec = _prevTask->timerSecTime;
    }
    _timerEndTime = QDateTime::currentDateTime().addSecs(intervalSec);
    return;
  }

  int seconds = (ms + 999) / 1000;
  ui->intervalTimer->setTime(QTime(0, 0).addSecs(seconds));
}

void MainWindow::startTimerMode() {
  if (!_prevTask || _prevTask->runMode != ERunMode::Timer) {
    return;
  }
  int intervalSec;
  if (_prevTask->timerSecTime == 0)
    intervalSec = ui->timerIntervalSpinBox->value();
  else {
    intervalSec = _prevTask->timerSecTime;
  }
  _timerEndTime = QDateTime::currentDateTime().addSecs(intervalSec);
  _tickTimer.start(TIMER_DISPLAY_UPDATE_TIME);

  updateTimerUi();
  logMessage(QString("Таймер запущен с интервалом %1 сек.").arg(intervalSec),
             Logger::Info);
}

void MainWindow::updateTimerUi() {
  qint64 ms = QDateTime::currentDateTime().msecsTo(_timerEndTime);

  if (ms < 0)
    ms = 0;

  int seconds = (ms + 999) / 1000;

  ui->intervalTimer->setTime(QTime(0, 0).addSecs(seconds));
}

void MainWindow::onStart() {
  if (isBusy()) {
    return;
  }
  _tickTimer.stop();
  ui->intervalTimer->setTime(QTime(0, 0));

  _remainingMs = 0;

  Task params = createTask();

  auto validationResult = validateTaskParams(params);
  if (validationResult != ETaskValidation::Ok) {
    handleValidationError(validationResult);
    return;
  }

  if (params.runMode == ERunMode::Timer && !validateTimerSettings()) {
    logMessage("Интервал таймера должен быть больше 0.", Logger::Error);
    return;
  }

  _prevTask.reset(new Task(std::move(params)));

  if (_prevTask->runMode == ERunMode::Timer) {
    startTimerMode();
  } else {
    _tickTimer.stop();
    runTask(false);
  }
}

void MainWindow::onPauseResume() {
  if (!_isPaused) {
    _remainingMs = QDateTime::currentDateTime().msecsTo(_timerEndTime);

    if (_worker) {
      logMessage("Запрос на паузу обработки.", Logger::Info);
      _worker->pause();
    }
  } else {
    _timerEndTime = QDateTime::currentDateTime().addMSecs(_remainingMs);
    _tickTimer.start(TIMER_DISPLAY_UPDATE_TIME);

    if (_worker) {
      logMessage("Запрос на возобновление работы.", Logger::Info);
      _worker->resume();
    }
  }
}

void MainWindow::onStop() {
  if (_worker) {
    logMessage("Запрос на остановку обработки.", Logger::Info);
    _worker->stop();
  }

  initStatusBars();

  _tickTimer.stop();

  _remainingMs = 0;
  _timerEndTime = QDateTime();

  ui->intervalTimer->setTime(QTime(0, 0, 0));
}

bool MainWindow::isPaused() {
  if (_worker)
    return _worker->isPaused();
  return false;
}

bool MainWindow::isBusy() {
  if (_worker)
    return _worker->isBusy();
  return true;
}
