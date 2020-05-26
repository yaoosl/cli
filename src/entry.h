#ifndef YS_CLI_ENTRY_H
#define YS_CLI_ENTRY_H

#pragma region utility macros
#ifndef CONCAT_
#define CONCAT_(L, R) L ## R
#endif // !CONCAT_
#ifndef CONCAT
#define CONCAT(L, R) CONCAT_(L, R)
#endif // !CONCAT
#ifndef STR_
#define STR_(IN) # IN
#endif // !STR_
#ifndef STR
#define STR(IN) STR_(IN)
#endif // !STR
#pragma endregion
#pragma region set environment info (x86/x64)
// Check windows
#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN64)
#define ENVIRONMENT x64
#define ENVIRONMENT64 ENVIRONMENT
#else
#define ENVIRONMENT x86
#define ENVIRONMENT32 ENVIRONMENT
#endif
#endif

// Check GCC
#if defined(__GNUC__)
#if defined(__x86_64__) || defined(__ppc64__)
#define ENVIRONMENT x64
#define ENVIRONMENT64 ENVIRONMENT
#else
#define ENVIRONMENT x86
#define ENVIRONMENT32 ENVIRONMENT
#endif
#endif

// Other Cases
#if !defined(ENVIRONMENT)
#define ENVIRONMENT NA
#endif
#pragma endregion

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 0
#define VERSION "RC1"
#define VERSION_FULL VERSION " - " __DATE__ " " __TIME__
#define VERSION_MAJORMINOR CONCAT(VERSION_MAJOR, VERSION_MINOR)


#endif // !YS_CLI_ENTRY_H