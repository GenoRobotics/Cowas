#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <Arduino.h>

#define MAINT_FILL_TIME 30000
#define MAINT_PURGE_TIME 27000

/**
    @brief: Disconnect the deployment module and put the connection into ethanol
*/
void purge_pipes_manifold();

/**
 * @brief: follow instructions, call function to know the times to run each pump
 * to put DNA shield into sterivex after sampling
*/
void calibrate_DNA_pump();

/**
 * @brief: steps to fill tube until T-connection with DNA shield
 */
void fill_DNA_shield_tube();


void system_checkup();


#endif