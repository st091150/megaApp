#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

// Параметры XOR-ключа
static constexpr int XOR_KEY_BYTES = 8;
static constexpr int HEX_KEY_LENGTH = XOR_KEY_BYTES * 2;

// Алгоритм только для ключей размера 8
static constexpr int ALGORITHM_KEY_SIZE = 8;

// Максимальная длина поля ввода для HEX-ключа
static constexpr int MAX_HEX_KEY_INPUT_LENGTH = HEX_KEY_LENGTH;

// Допустимые символы для HEX-ключа
static const char *HEX_VALID_CHARS = "0123456789ABCDEFabcdef";

// Pattern для label обработки
static const char *PROCESSING_LABEL_PLACEHOLDER = "Обработано";
static const char *PROCESSING_LABEL_TEMPLATE = "Обработано %1 из %2";
static const char *FILE_PROCESSING_LABEL_TEMPLATE = "Процесс обработки %1";

// Pattern для HEX-ключа
inline QString hexPattern() {
  static const QString pattern =
      QString("^[%1]{%2}$").arg(HEX_VALID_CHARS).arg(HEX_KEY_LENGTH);
  return pattern;
}

// PlaceHolder для поля HEX-ключа
inline QString hexKeyPlaceholder() {
  return QString("Введите %1 hex-символов").arg(HEX_KEY_LENGTH);
}

// Ограничения опроса входных файлов
static constexpr int MIN_TIMER_INTERVAL = 1;

// Обновление таймера (мс)
static constexpr int TIMER_DISPLAY_UPDATE_TIME = 200;

// Ошибки Формирования задачи
static const char *UNKNOWN_TASK_PARAMS_ERROR_MSG =
    "Произошла неизвестная ошибка.";
static const char *INPUT_PATH_EMPTY_ERROR_MSG = "Входная папка не указана.";
static const char *INPUT_FOLDER_DOES_NOT_EXIST_MSG =
    "Входная папка не существует.";
static const char *OUTPUT_EMPTY_PATH_ERROR_MSG = "Выходная папка не указана.";
static const char *OUTPUT_PATH_CREATE_ERROR_MSG =
    "Не удалось создать выходную папку.";

inline QString invalidHexKeyErrorMsg() {
  static const QString INVALID_HEX_KEY_ERROR_MSG =
      QString("HEX ключ должен быть ровно %1 байт (%2 hex-символов).")
          .arg(XOR_KEY_BYTES)
          .arg(MAX_HEX_KEY_INPUT_LENGTH);
  return INVALID_HEX_KEY_ERROR_MSG;
}

//
// WORKER THREAD
// Размер блока для чтения/записи файлов
static constexpr int CHUNK_SIZE = 4 * 1024 * 1024; // 4 МБ

// Описание событий от воркера
static const char *UNKNOWN_EVENT_MSG = "Неизвестное событие";

static const char *START_TASK_MSG = "Начало выполнения задачи";
static const char *PAUSED_TASK_MSG = "Пауза выполнения";
static const char *RESUMED_TASK_MSG = "Возобновление выполнения";
static const char *STOPPED_TASK_MSG = "Остановка выполнения";
static const char *FINISHED_TASK_MSG = "Обработка файлов завершена";

static const char *STARTED_PROCESS_FILE_MSG = "Начало обработки файла";
static const char *FILE_PROGRESS_MSG = "Прогресс обработки файла";
static const char *FILE_FINISHED_MSG = "Обработка файла завершена";

static const char *USER_STOP_REQUEST_MSG =
    "Завершение выполнение пользователем";
static const char *USER_PAUSE_REQUEST_MSG = PAUSED_TASK_MSG;
static const char *USER_RESUME_REQUEST_MSG = RESUMED_TASK_MSG;

static const char *PROCESSING_ERROR_MSG = "Ошибка обработки";

inline QString hexKeySizeErrorMsg() {
  static const QString HEX_KEY_SIZE_ERROR =
      QString("Неверный размер ключа (ожидаемый размер %1)")
          .arg(ALGORITHM_KEY_SIZE);
  return HEX_KEY_SIZE_ERROR;
}

static const char *OPEN_INPUT_FOLDER_ERROR_MSG =
    "Не удалось открыть папку поиска входных файлов";
static const char *OPEN_OUTPUT_FOLDER_ERROR_MSG =
    "Не удалось открыть папку для записи результирующих файлов";
static const char *HEX_KEY_EMPTY_MSG = "Ключ не задан";
static const char *READ_FILE_UNKNOWN_ERROR_MSG =
    "Не удалось прочитать данные из файла";
static const char *FILE_WRITE_UNKNOWN_ERROR_MSG =
    "Не удалось записать данные в файла";
static const char *RENAME_FILE_ERROR_MSG = "Не удалось заменить файл";
static const char *CANT_DELETE_INPUT_FILE_MSG =
    "Не удалось удалить входной файл";
static const char *TEMP_FILE_DELETE_ERROR_MSG =
    "Не удалось удалить старый файл";

#endif // CONFIG_H
