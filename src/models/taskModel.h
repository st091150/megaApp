#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QString>
#include <QStringList>
#include <QMetaType>

namespace TaskModel {
    enum class EDuplicateAction {
        Overwrite,
        AddCounter
    };

    enum class ERunMode {
        Once,
        Timer
    };

    struct Task {
        QString inputPath;
        QString outputPath;
        QString fileMask;

        QStringList files;
        QByteArray hexKey;

        bool deleteSourceFlag = false;
        int timerSecTime = 0;

        EDuplicateAction duplicateAction = EDuplicateAction::Overwrite;
        ERunMode runMode = ERunMode::Once;
    };

} // namespace TaskModel

Q_DECLARE_METATYPE(TaskModel::Task)

#endif // TASKMODEL_H
