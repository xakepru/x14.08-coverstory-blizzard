#ifndef DEBUG_MOD
#define DEBUG_MOD

#define _CRT_SECURE_NO_WARNINGS

// enable debug information (main switch)
#define ALLOW_DEBUG_OUTPUT

// types of debug output
#define DBG_STRINGS 0
#define DBG_FILE	1
#define DBG_BUFF	2

// select type
#define DBG_OUT DBG_STRINGS

void DbgOut( char * msg, ... );

#endif