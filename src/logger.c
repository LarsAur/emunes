#include <windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <string.h>
#include <stdio.h>
#include "logger.h"

HANDLE fileHandle;

void CreateLogFile()
{
    fileHandle = CreateFileA(
        "emunes.log",
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Unable to create log file", "Warning", MB_ICONWARNING | MB_OK);
    }
}

void CloseLogFile()
{
    CloseHandle(fileHandle);
}

void Log(char *msg, LOG_LEVEL ll)
{
    if(ll < CURRENT_LOG_LEVEL) return;

    DWORD lpBytesWritten;

    SYSTEMTIME localTime;
    GetLocalTime(&localTime);

    char logPrefix[100];
    sprintf(logPrefix, "[%02d:%02d:%02d %02d:%02d:%d] ", localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wDay, localTime.wMonth, localTime.wYear);

    switch (ll)
    {
    case LL_DEBUG:
        strcat(logPrefix, "[DEBUG]\t");
        break;
    case LL_INFO:
        strcat(logPrefix, "[INFO]\t");
        break;
    case LL_WARNING:
        strcat(logPrefix, "[WARNING]\t");
        break;
    case LL_ERROR:
        strcat(logPrefix, "[ERROR]\t");
        break;
    }

    WriteFile(fileHandle, logPrefix, strlen(logPrefix), &lpBytesWritten, NULL);
    WriteFile(fileHandle, msg, strlen(msg), &lpBytesWritten, NULL);
    WriteFile(fileHandle, "\n", 1, &lpBytesWritten, NULL);
}

void Logf(char *msg, LOG_LEVEL ll, ...)
{
    va_list args;
    va_start(args, ll);

    char strbuf[4096] = {0};
    _vsnprintf_s(strbuf, sizeof(strbuf), _TRUNCATE, msg, args);

    va_end(args);

    Log(strbuf, ll);
}

char *opcode_to_string[] =
    {
        "ADC",
        "AND",
        "ASL",
        "BCC",
        "BCS",
        "BEQ",
        "BIT",
        "BMI",
        "BNE",
        "BPL",
        "BRK",
        "BVC",
        "BVS",
        "CLC",
        "CLD",
        "CLI",
        "CLV",
        "CMP",
        "CPX",
        "CPY",
        "DEC",
        "DEX",
        "DEY",
        "EOR",
        "INC",
        "INX",
        "INY",
        "JMP",
        "JSR",
        "LDA",
        "LDX",
        "LDY",
        "LSR",
        "NOP",
        "ORA",
        "PHA",
        "PHP",
        "PLA",
        "PLP",
        "ROL",
        "ROR",
        "RTI",
        "RTS",
        "SBC",
        "SEC",
        "SED",
        "SEI",
        "STA",
        "STX",
        "STY",
        "TAX",
        "TAY",
        "TSX",
        "TXA",
        "TXS",
        "TYA",
        "NIL",
};
