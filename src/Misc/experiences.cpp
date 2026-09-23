#include <Arduino.h>
#include "C_output.h"

#include "experiences.h"

#include "Button.h"
#include "C_output.h"
#include "Settings.h"
#include "Step_functions.h"

// Delay when actuating valves
#define DELAY_ACTIONS 1000

extern C_output output;
extern Button button_start;
extern Button button_container;
extern Button button_spool_up;
extern Button button_spool_down;
extern Button button_left;
extern Button button_right;
extern struct Timer timer_control_pressure1;

void cross_cont_exp(){
    output.println("Cross contamination test started");
    output.println("");
    output.println("Reuse the purged water in recipient, to filter and reuse later");
    output.println("When pipe is destilled water, press start");

    // using 4 liter of distilled water

    button_start.waitPressedAndReleased();

    // sample 1, distilled water
    // !!!!!!!! -----------------
    step_fill_container();
    step_purge(false);
    // !! ------------


    output.println("REUSE filtered water in 5L container");
    output.println("When pipe is in NEW destilled water, press start to start SAMPLE N°1");

    button_start.waitPressedAndReleased();

    step_fill_container();
    step_sampling(1, false);

    // sample 2, no need to rinse as sample before was with destilled water
    output.println("Put filtered water in seperate recipient to discard colony later"); // !
    output.println("When pipe is in bacterial colony, press start for SAMPLING N°2");

    button_start.waitPressedAndReleased();
    
    step_fill_container();
    step_sampling(2, false);

    output.println("");
    output.println("Discard the water with the bacterial colony");
    

    output.println("Reuse water in a recipient");
    output.println("When pipe is in distilled water container 5L (2L, filtered from sample 1), press start");
    button_start.waitPressedAndReleased();

    // rinsing bacterial colony, before sample 3

    step_fill_container();
    delay(3000);
    step_purge(false);

    output.println("Filter the 4L of water (from last purge and first purge) through a serivex into the 5L container");
    output.println("When filtered, put pipe into the 5L container (containing 4L of distilled water)");
    output.println("Press start button when ready");

    button_start.waitPressedAndReleased();

    for(uint8_t i = 0; i < 2; i++){
        step_fill_container();
        delay(3000);
        step_purge(false);
    }



    output.println("Purge finished");
    output.println("When pipe is in NEW (the last 2L) distilled water, press start for SAMPLE N°3");
    button_start.waitPressedAndReleased();

    step_fill_container();
    step_sampling(3, false);
}


void exp_explore_2905(){
    output.println("Start of experiment - L'EXPLORE");
    output.println("--------------------------------------------");

    // output.println("Start of sample 1 in slot 1");
    // output.println("Depth : 39m");
    // output.println("Press start when ready");
    // button_start.waitPressedAndReleased();
    // sample_process(39*100, 1);

    output.println("Start of sample 2 in slot 4");
    output.println("Depth : 5m");
    output.println("Press start when ready");
    button_start.waitPressedAndReleased();
    sample_process(5*100, 4);

    output.println("Start of sample 3 in slot 8");
    output.println("Depth : 5m");
    output.println("Press start when ready");
    button_start.waitPressedAndReleased();
    sample_process(5*100, 8);

    output.println("Start of sample 4 in slot 11");
    output.println("Depth : 5m");
    output.println("Press start when ready");
    button_start.waitPressedAndReleased();
    sample_process(5*100, 11);

    output.println("Start of sample 5 in slot 14");
    output.println("Depth : 3m");
    output.println("Press start when ready");
    button_start.waitPressedAndReleased();
    sample_process(3*100, 14);

    output.println("Well done, get a drink and enjoy the succesfull experiene");
    output.println("------------------------------------------------------------");
}