#include "LogHandler.h"

// Initialize static global log level
LogLevel LogHandler::_globalLogLevel = LOG_LEVEL_INFO;

bool LogHandler::shouldLog(LogLevel level) const {
    // Check both instance level and global level
    LogLevel effectiveLevel = (_logLevel < _globalLogLevel) ? _logLevel : _globalLogLevel;
    return level <= effectiveLevel;
}

void LogHandler::setLogLevel(LogLevel level) {
    _logLevel = level;
}

LogLevel LogHandler::getLogLevel() const {
    return _logLevel;
}

void LogHandler::setGlobalLogLevel(LogLevel level) {
    _globalLogLevel = level;
}

LogLevel LogHandler::getGlobalLogLevel() {
    return _globalLogLevel;
}

const char* LogHandler::logLevelToString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_NONE: return "NONE";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_WARN: return "WARN";
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_VERBOSE: return "VERBOSE";
        default: return "UNKNOWN";
    }
}

LogLevel LogHandler::stringToLogLevel(const char* levelStr) {
    if (strcmp(levelStr, "NONE") == 0) return LOG_LEVEL_NONE;
    if (strcmp(levelStr, "ERROR") == 0) return LOG_LEVEL_ERROR;
    if (strcmp(levelStr, "WARN") == 0) return LOG_LEVEL_WARN;
    if (strcmp(levelStr, "INFO") == 0) return LOG_LEVEL_INFO;
    if (strcmp(levelStr, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
    if (strcmp(levelStr, "VERBOSE") == 0) return LOG_LEVEL_VERBOSE;
    return LOG_LEVEL_INFO; // Default fallback
}

void LogHandler::print_buffer(const uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) return;
    for (size_t i = 0; i < length; i++) {
        Serial.printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) Serial.printf("\n");
    }
    Serial.printf("\n");
}

void LogHandler::setVerbose(bool verbose) {
    _verbose = verbose;
}

const char* LogHandler::Error(esp_err_t errCode) {
    // convert esp error code to text
    static char errText[100];
    esp_err_to_name_r(errCode, errText, 100);
    return errText;
}

void LogHandler::info(const char* format, ...) {
    if (!shouldLog(LOG_LEVEL_INFO)) return;
    
    va_list args;
    va_start(args, format);
    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    Serial.printf("[%s %s] %s\n", 
        esp_log_system_timestamp(), _tag, buff);
    va_end(args);
}

void LogHandler::warn(const char* format, ...) {
    if (!shouldLog(LOG_LEVEL_WARN)) return;
    
    va_list args;
    va_start(args, format);
    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    Serial.printf(LOG_COLOR_W "[%s WARN %s] %s" LOG_RESET_COLOR "\n",
        esp_log_system_timestamp(), _tag, buff);
    va_end(args);
}

void LogHandler::error(const char* format, ...) {
    if (!shouldLog(LOG_LEVEL_ERROR)) return;
    
    va_list args;
    va_start(args, format);
    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    Serial.printf(LOG_COLOR_E "[%s ERROR %s] %s" LOG_RESET_COLOR "\n",
        esp_log_system_timestamp(), _tag, buff);
    va_end(args);
}

void LogHandler::verbose(const char* format, ...) {
    if (!_verbose || !shouldLog(LOG_LEVEL_VERBOSE)) return;
    
    va_list args;
    va_start(args, format);
    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    Serial.printf(LOG_COLOR_I "[%s VERBOSE %s] %s" LOG_RESET_COLOR "\n",
        esp_log_system_timestamp(), _tag, buff);
    va_end(args);
}

void LogHandler::debug(const char* format, ...) {
    if (!shouldLog(LOG_LEVEL_DEBUG)) return;
    
    va_list args;
    va_start(args, format);
    char buff[256];
    vsnprintf(buff, sizeof(buff), format, args);
    Serial.printf(LOG_COLOR_D "[%s ### DEBUG %s] %s" LOG_RESET_COLOR "\n",
        esp_log_system_timestamp(), _tag, buff);
    va_end(args);
    delay(100);
}
