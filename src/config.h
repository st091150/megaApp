#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

// Параметры XOR-ключа
static constexpr int XOR_KEY_BYTES = 8;
static constexpr int HEX_KEY_LENGTH = XOR_KEY_BYTES * 2;

// Алгоритм только для ключей размера 8
static constexpr int ALGORITHM_HEX_KEY_SIZE = 8;

// Допустимые символы для HEX-ключа
static const char *HEX_VALID_CHARS = "0123456789ABCDEFabcdef";

// Максимальная длина поля ввода для HEX-ключа
static constexpr int MAX_HEX_KEY_INPUT_LENGTH = HEX_KEY_LENGTH;

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

// taskParams ограничения
static constexpr int MIN_TIMER_INTERVAL = 1;

// Обновление таймер (мс)
static constexpr int TIMER_DISPLAY_UPDATE_TIME = 200;

// WORKER THREAD
// Размер блока для чтения/записи файлов
static constexpr int CHUNK_SIZE = 4 * 1024 * 1024; // 4 МБ

// Сообщения от воркера
static const char *USER_STOP_REQUEST = "Завершение работы пользователем";
static const char *TASK_FINISHED = "Обработка файлов завершена";
static const char *PROCESSING_ERROR = "Ошибка обработки";

static const char *OPEN_INPUT_FOLDER_ERROR =
    "Не удалось открыть папку поиска входных файлов";
static const char *OPEN_OUTPUT_FOLDER_ERROR =
    "Не удалось открыть папку для записи результирующих файлов";
static const char *HEX_KEY_EMPTY = "Ключ не задан";
static const char *READ_FILE_UNKNOWN_ERROR =
    "Не удалось прочитать данные из файла";
static const char *FILE_WRITE_UNKNOWN_ERROR =
    "Не удалось записать данные в файла";
static const char *OLD_FILE_DELETE_ERROR = "Не удалось удалить старый файл";
static const char *RENAME_FILE_ERROR = "Не удалось заменить файл";
static const char *CANT_DELETE_INPUT_FILE = "Не удалось удалить входной файл";

#endif // CONFIG_H
