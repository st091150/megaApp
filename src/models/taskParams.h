#ifndef TASKPARAMS_H
#define TASKPARAMS_H

#include <QString>
#include <QStringList>
#include <QMetaType>

struct TaskParams {
    enum class DuplicateAction {
        Overwrite,
        AddCounter
    };

    enum class RunMode {
        Once,
        Timer
    };

public:
    QString inputPath;
    QString outputPath;
    QString fileMask;

    QStringList files;
    QByteArray hexKey;

    bool deleteSourceFlag = false;
    int timerSecTime = 0;

    DuplicateAction duplicateAction = DuplicateAction::Overwrite;
    RunMode runMode = RunMode::Once;
};

Q_DECLARE_METATYPE(TaskParams)

#endif // TASKPARAMS_H
