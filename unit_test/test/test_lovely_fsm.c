/* --------------------------------------------------------------------------
 * For this unit test we will simulate a temperature supervisor, that will
 * trigger an alarm on overtemperature and has to be reset by a keypress, once
 * the temperature has gone down.
 * -------------------------------------------------------------------------- */

#include "unity.h"
#include "../../src/lovely_fsm.h"

// -------- HELPERS -------------------------------------
#define MAX_TEMP    100
int temperature;

// -------- STATE MACHINE SETUP -------------------------
enum events {
    EV_BUTTON_PRESS,
    EV_TEMP_OVER
};

enum states {
    ST_IDLE,
    ST_ALARM
};

int temperature_okay(int threshold);

lfsm_transitions_t transition_table[] = {
    { ST_IDLE,  EV_BUTTON_PRESS, 1, ST_IDLE },
    { ST_IDLE,  EV_TEMP_OVER   , 1, ST_ALARM },
    { ST_ALARM, EV_BUTTON_PRESS, temperature_okay(100), ST_IDLE },
}

// -------- STATE MACHINE CALLBACKS ----------
int temperature_okay(int threshold) {
    return temperature < MAX_TEMP;
}


// -------- UNITY SETUP AND TEARDOWN CALLBACKS ----------
void setUp(void) {

}

void tearDown(void) {

}

void test_ignore(void) {
    TEST_IGNORE();
}
