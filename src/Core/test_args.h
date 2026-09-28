#ifndef CORE_TEST_ARGS_H
#define CORE_TEST_ARGS_H

// Arguments of the FW2 console test commands, e.g.
//   rotate_all dry from=3 count=4 order=index hold=5 auto
// Pure parsing, no Arduino.h, unit-tested with `pio test -e native` (test/test_test_args/).

#include "slot_iteration.h"

namespace core
{

const int ARGS_MAX_TOKENS = 12;
const int ARGS_TOKEN_LEN = 24; // including the terminating 0

// Tokens after the command word. A token "k=v" is a key/value pair; anything else is a
// positional (key[i] is then empty and value[i] holds the token).
struct Args
{
    int count;
    char key[ARGS_MAX_TOKENS][ARGS_TOKEN_LEN];
    char value[ARGS_MAX_TOKENS][ARGS_TOKEN_LEN];
};

enum ArgsError
{
    ARGS_OK = 0,
    ARGS_TOO_MANY,       // more than ARGS_MAX_TOKENS tokens
    ARGS_TOKEN_TOO_LONG, // a key or a value does not fit ARGS_TOKEN_LEN
    ARGS_EMPTY_KEY,      // "=5"
    ARGS_EMPTY_VALUE,    // "count="
    ARGS_DUPLICATE_KEY   // "count=2 count=3"
};

const char *args_error_name(ArgsError e);

// Splits `line` on spaces and tabs (a trailing CR/LF is ignored). The first word goes to
// `command` (truncated to command_cap - 1 characters, empty line = empty command).
ArgsError parse_args(const char *line, char *command, int command_cap, Args *out);

int args_positional_count(const Args &a);                // positionals that are not in `flags`
const char *args_positional(const Args &a, int i);       // i-th positional (any), 0 if none
bool args_has_flag(const Args &a, const char *word);     // a positional equal to `word`
const char *args_value(const Args &a, const char *key);  // value of key=..., 0 if absent

// Index of the first token the command does not accept, or -1: a key not in `keys`, or
// more than `max_positionals` positionals that are not one of `flags`.
int args_first_unknown(const Args &a, const char *const *keys, int nkeys,
                       const char *const *flags, int nflags, int max_positionals);

// Strict number parsing: the whole string must be the number.
bool parse_long_strict(const char *s, long *out);   // [+-]digits, at most 9 digits
bool parse_float_strict(const char *s, float *out); // [+-]digits[.digits] or .digits
bool parse_yes_no(const char *s, bool *out);        // y, yes, n, no (any case)

// "50" = 50 mL; "20s" = 20 seconds. Exactly one of the outputs is set (> 0).
bool parse_ml_or_seconds(const char *s, float *ml, long *seconds);

// from=N (a slot number), count=M, order=index|fill -> a SlotSelection (starting from
// `base`). Returns false and sets *error to a short reason on a bad value.
bool selection_from_args(const Args &a, SlotSelection base, SlotSelection *out, const char **error);

} // namespace core

#endif
