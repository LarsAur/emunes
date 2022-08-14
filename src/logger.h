#ifndef LOGGER_H

#define LOGGER_H


typedef enum LOG_LEVEL
{
    LL_DEBUG = 0,
    LL_INFO = 1,
    LL_WARNING = 2,
    LL_ERROR = 3,
}LOG_LEVEL;

#define CURRENT_LOG_LEVEL LL_INFO

void CreateLogFile();

void CloseLogFile();

void Log(char *msg, LOG_LEVEL ll);

void Logf(char *msg, LOG_LEVEL, ...);

extern char *opcode_to_string[57];

#endif