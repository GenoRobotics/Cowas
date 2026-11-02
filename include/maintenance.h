#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <Arduino.h>

/**
 * @brief: follow instructions, call function to know the times to run each pump
 * to put DNA shield into sterivex after sampling
 */
void calibrate_DNA_pump();

/**
 * @brief Ethanol cleaning of the tubing (console `ethanol_purge`). No filter installed, inlet and outlet in 70 % ethanol.
 * Warns, asks which slots were unused (or reads them from `args`, e.g. "skip=3,7,12"), then: prime through the purge slot
 * (3 L with the spool module, else 0.5 L), 0.5 L through each used slot at full power, 30 s of air through each, and 1 min of
 * air at the purge slot. Settings: CLEAN_* in Settings.h.
 */
void purge_pipes_manifold(const char *args = nullptr);

/**
 * @brief: steps to fill tube until T-connection with DNA shield
 */
void fill_DNA_shield_tube();

void system_checkup();

#endif