#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "taskModel.h"
#include "utils.h"
#include "worker.h"

#include <QDateTime>
#include <QMainWindow>
#include <QTimer>

using namespace TaskModel;
using namespace WorkerModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MegaApp;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  using OutputMode = Logger::EOutputMode;
  using LogLevel = Logger::ELogLevel;

  using Task = TaskModel::Task;

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
  Task createTask() const;

  bool validateTimerSettings() const;
  void handleValidationError(ETaskValidation result);

  void runTask(bool fromTimer);

  void startProcessing(const Task &task);

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

  void logTask(const TaskModel::Task &task);
  void parseWorkerStatus(const Status &s);

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

  QScopedPointer<Task> _prevTask;
  QScopedPointer<Logger> logger;

  QTimer _tickTimer;
  QDateTime _timerEndTime;

  qint64 _remainingMs;
  bool _isPaused;

  QThread *_thread;
  Worker *_worker;
};

#endif // MAINWINDOW_H
