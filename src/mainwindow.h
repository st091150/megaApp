#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "errorMsg.h"
#include "logger.h"
#include "taskParams.h"
#include "worker.h"
#include "workerStatus.h"


#include <QDateTime>
#include <QMainWindow>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui {
class MegaApp;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  using OutputMode = Logger::OutputMode;
  using LogLevel = Logger::LogLevel;

private:
  void init();

  void initStatusBars();
  void updateStatusBars(int processedFileCount, int totalFiles,
                        int processingPercentage,
                        const QString &processingFileName,
                        int fileProcessingPercentage);

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  TaskParams createTaskParams() const;

  ParamsValidationResult validateTaskParams(const TaskParams &params) const;
  bool validateTimerSettings() const;
  void handleValidationError(ParamsValidationResult result);

  void runTask(bool fromTimer);

  void startProcessing(const TaskParams &task);

  void logMessage(const QString &msg, LogLevel logLevel);

  void logMessage(const QString &msg, LogLevel logLevel, OutputMode mode);
  inline void infoLogMessage(const QString &msg) {
    logMessage(msg, LogLevel::Info);
  }
  inline void warningLogMessage(const QString &msg) {
    logMessage(msg, LogLevel::Warning);
  }
  inline void errorLogMessage(const QString &msg) {
    logMessage(msg, LogLevel::Error);
  }

  void logTask(const TaskParams &params);
  void parseWorkerStatus(const WorkerStatus &workerStatus);

private slots:
  void browseInputDirectory();
  void browseOutputDirectory();

  void onTick();
  void updateTimerUi();
  void startTimerMode();

  void onStart();
  void onPauseResume();
  void onStop();

  bool isPaused();
  bool isBusy();

private:
  Ui::MegaApp *ui;

  QScopedPointer<TaskParams> _prevTaskParams;
  QScopedPointer<Logger> logger;

  QTimer _tickTimer;
  QDateTime _timerEndTime;

  qint64 _remainingMs;
  bool _isPaused;

  QThread *_thread;
  Worker *_worker;
};

#endif // MAINWINDOW_H
