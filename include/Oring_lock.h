#ifndef ORING_LOCK_H
#define ORING_LOCK_H

#include <Arduino.h>

bool lock_oring();
void unlock_oring();
bool is_oring_locked();

#endif
