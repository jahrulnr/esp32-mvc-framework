#pragma once

#include <Arduino.h>
#include <cstdarg>
#include <esp_log.h>

/**
 * Log levels in order of severity (lowest to highest):
 * NONE - No logging
 * ERROR - Critical errors only
 * WARN - Warnings and errors
 * INFO - General information, warnings, and errors (default)
 * DEBUG - Debug information and all above
 * VERBOSE - All messages including verbose debug
 */
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
};

class LogHandler {
public:
    LogHandler(const char* tag): _tag(tag), _verbose(false), _logLevel(LOG_LEVEL_INFO){};
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void verbose(const char* format, ...);
    void debug(const char* format, ...);

    void print_buffer(const uint8_t* buffer, size_t length);
    void setVerbose(bool verbose);
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    const char* Error(esp_err_t errCode);

    // Static global log level control
    static void setGlobalLogLevel(LogLevel level);
    static LogLevel getGlobalLogLevel();
    static const char* logLevelToString(LogLevel level);
    static LogLevel stringToLogLevel(const char* levelStr);

private:
    const char* _tag;
    bool _verbose;
    LogLevel _logLevel;
    static LogLevel _globalLogLevel;
    
    bool shouldLog(LogLevel level) const;
};
