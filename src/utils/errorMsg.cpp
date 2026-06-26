#include "errorMsg.h"
#include "config.h"

QString errorMessage(ParamsValidationResult code) {
    switch (code) {
        case ParamsValidationResult::Ok:
            return QString();
        case ParamsValidationResult::InputPathEmpty:
            return "Входная папка не указана.";
        case ParamsValidationResult::InputPathNotExists:
            return "Входная папка не существует.";
        case ParamsValidationResult::OutputPathEmpty:
            return "Выходная папка не указана.";
        case ParamsValidationResult::OutputPathCannotCreate:
            return "Не удалось создать выходную папку.";
        case ParamsValidationResult::InvalidXorKey:
            return QString("HEX ключ должен быть ровно %1 байт (%2 hex-символов).").arg(XOR_KEY_BYTES).arg(MAX_HEX_KEY_INPUT_LENGTH);
        default:
            return "Произошла неизвестная ошибка.";
    }
}

QString workerStateToString(WorkerStatus::State state){
    switch (state) {
        case WorkerStatus::State::Idle:
            return "Ожидание";
        case WorkerStatus::State::Running:
            return "Выполняется";
        case WorkerStatus::State::Paused:
            return "Пауза";
        case WorkerStatus::State::Stopped:
            return "Остановлено";
        case WorkerStatus::State::Finished:
            return "Завершено";
        case WorkerStatus::State::Error:
            return "Ошибка";
        default:
            return "Неизвестно";
    }
}
QString workerEventToString(WorkerStatus::Event event) {
    switch (event) {
        case WorkerStatus::Event::TaskStarted:
            return "Начало выполнения задачи";

        case WorkerStatus::Event::TaskPaused:
            return "Приостановка выполнения";

        case WorkerStatus::Event::TaskError:
            return "Ошибка выполнения задачи";

        case WorkerStatus::Event::TaskStopped:
            return "Остановка выполнения задачи";

        case WorkerStatus::Event::TaskFinished:
            return "Задача заверешна";

        case WorkerStatus::Event::TaskResumed:
            return "Возобновление задачи";

        case WorkerStatus::Event::FileStarted:
            return "Начало обработки нового файла";

        case WorkerStatus::Event::FileProgressUpdate:
            return "Обновление статуса задачи";

        case WorkerStatus::Event::FileError:
            return "Ошибка обработки файла";

        case WorkerStatus::Event::FileFinished:
            return "Обработка файла завершена";

        case WorkerStatus::Event::None:
        default:
            return "Неизвестно";
    }
}
