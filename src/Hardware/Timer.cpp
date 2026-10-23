/**
 * @file Timer.cpp
 * @author Timothée Hirt & Paco Mermoud
 * @brief See Cortex M3 datasheet.
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "Timer.h"  

// ===== Timer list =====
// ISR/IRQ	TC	Channel	Due pins
// TC0	TC0	0	2, 13
// TC1	TC0	1	60, 61
// TC2	TC0	2	58
// TC3	TC1	0	none
// TC4	TC1	1	none
// TC5	TC1	2	none
// TC6	TC2	0	4, 5
// TC7	TC2	1	3, 10
// TC8	TC2	2	11, 12

// TC3_HANDLER() located in pressure sensor class

