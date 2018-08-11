// uLog logging system by gatopeich (c) 2018
//
// Features:
// - Single header
// - Per-message, per level enabling/disabling
// - Output to stderr/file
// - Printf formatting
// - TBD: Output to syslog/udp/tcp
// - TBD: Per-message throttling
// - TBD: Background thread to offload work
// - TBD: Multiple sinks?
//
// Design goals
// - GCC+C11 compatibility
// - Minimal cache misses on disabled messages

#ifndef __ULOG__
#define __ULOG__ 0.01 // Version

#include <stdio.h>
#include <regex.h>

// Compiler specific definitions (GCC only)
#define SHARED __attribute__((weak))
#define STARTUP __attribute__((constructor))
#define SHUTDOWN __attribute__((destructor))


#ifdef ULOG_QUIET
#define ULOG_INTERNAL(...) // Disable uLog's own messages
#else
#define ULOG_INTERNAL ULOG
#endif


// Registry entry kept for each uLog message instance
typedef struct uLog_entry {
    char                enabled; // TBD: Optimize in terms if cache misses
    const char*         message; // TBD: Store these pointers out of the statically-allocated space?
    struct uLog_entry*  next;    // TBD: A more efficient container?
} uLog_entry;

// Shared uLog data
SHARED struct {
    uLog_entry* registry;
    FILE*       file;
    int         level;
} uLog_ = {NULL, NULL, 5};


// Global initializer
STARTUP SHARED void uLog_init(void)
{
    uLog_.file = stderr;
}

SHARED void uLog_set_file(FILE* file) { uLog_.file = file; }

// The essential definition, implemented in most basic terms:
#define ULOG(_message, _args...) do{ \
    static uLog_entry _entry = { 0, _message }; \
    STARTUP void uLog_entry_init(void) { uLog_register(&_entry); } \
    if (_entry.enabled && uLog_.file) \
        fprintf(uLog_.file, _message "\n", ##_args); \
}while(0)


// List of all registered log entries

// Syslog levels: [0]Emergency, [A]lert, [C]ritical, [E]rror, [W]arning, [N]otice, [I]nfo, [D]ebug
#define ULOG_LEVELS "0acewnid"
// First char in the message is used to convey its priority, e.g.: "N: Starting up..."
SHARED int uLog_level_of(const char* message)
{
    int level = 0;
    while (ULOG_LEVELS[level] != (message[0]|('a'-'A'))) // ASCII case-insensitive comparison
        if (!ULOG_LEVELS[++level])
            return -1; // Invalid level specifier
    return level;
}

SHARED void uLog_set_level(int level)
{
    uLog_.level = level;

    #define ULOG_FOREACH(action) \
        for (uLog_entry *entry = uLog_.registry; entry; entry = entry->next) { action; }
    ULOG_FOREACH(entry->enabled = uLog_level_of(entry->message) <= uLog_.level);
}

// Due to an issue in GCC nested constructor functions, we cannot guarantee order of initialization
SHARED void uLog_register(uLog_entry *entry)
{
    entry->enabled = uLog_level_of(entry->message) <= uLog_.level;
    entry->next = uLog_.registry;
    uLog_.registry = entry;
    // ULOG_INTERNAL("Registered %p %.20s with level %d", entry, entry->message, uLog_level_of(entry->message));
}

SHARED int uLog_enable_by_regex(const char* regex, int enabled)
{
    regex_t preg;
    int err = regcomp(&preg, regex, REG_EXTENDED|REG_NOSUB);
    if (err) {
        ULOG("e: bad regex expression");
        return -1;
    }
    int n = 0;
    ULOG_FOREACH(
        if (regexec(&preg, entry->message, 0, NULL,0) != REG_NOMATCH) {
            ++n; entry->enabled = enabled;
        }
    );
    regfree(&preg);
    return n;
}

SHARED SHUTDOWN void uLog_report(uLog_entry *entry)
{
    int n_entries = 0, n_enabled = 0;
    ULOG_FOREACH(++n_entries; n_enabled += entry->enabled);
    ULOG_INTERNAL("i: uLog v%g: uLog_.level = %d, n_entries = %d, n_enabled = %d"
        , __ULOG__, uLog_.level, n_entries, n_enabled);
}


#endif // __ULOG__
