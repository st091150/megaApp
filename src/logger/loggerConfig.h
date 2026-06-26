#ifndef LOGGERCONFIG_H
#define LOGGERCONFIG_H

#include <QString>

// Папка логов
static const char *BASE_LOG_FOLDER = "/logs";

// Название файлов логов
static const char *BASE_LOG_FILE_NAME = "appLog";

// Максимальный размер файла логов
static constexpr int MAX_FILE_SIZE = 4 * 1024 * 1024; // 4 MB

// Максимальное количество файлов логов
static constexpr int MAX_FILE_COUNT = 3;

#endif // LOGGERCONFIG_H
