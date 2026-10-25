#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <Arduino.h>

/**
 * @brief: follow instructions, call function to know the times to run each pump
 * to put DNA shield into sterivex after sampling
 */
void calibrate_DNA_pump();

/**
    @brief: Disconnect the deployment module and put the connection into ethanol
*/
void purge_pipes_manifold();

/**
 * @brief: steps to fill tube until T-connection with DNA shield
 */
void fill_DNA_shield_tube();

void system_checkup();

#endif