/* --------------------------------------------------------------------------
 * For this unit test we will simulate a temperature supervisor, that will
 * trigger an alarm on overtemperature and has to be reset by a keypress, once
 * the temperature has gone down.
 * -------------------------------------------------------------------------- */

#include "unity.h"
#include <stdio.h>
#include "../../src/lovely_fsm.h"
#include "../../lovelyBuffer/buf_buffer.h"
// include last!
#include "../../src/lovely_fsm_debug.c"
// -------- User data structures -------------------------------------
enum {
    LED_OFF,
    LED_ON
};

typedef struct my_data_t {
    int16_t temperature;
    uint8_t red_led;
    uint8_t yellow_led;
    uint8_t green_led;
} my_data_t;
my_data_t my_data;

#define WARN_TEMP    80
#define ALARM_TEMP  100

// -------- STATE MACHINE SETUP -------------------------
// The state machine setup consists of the following steps:
// 1. define states and events (e.g. enum for readability/convenience)
// 2. create a lfsm_transitions_t[] to create the state machine transitions
// 3. create a lfsm_state_functions_t[] to describe the function of each state
// 4. write the condition functions (return int, as boolean)
//    These functions will be evaluated when an event is triggered.
//    Make sure that condition functions do not overlap!
//    When condition 1 is: temp>100 and condition 2 is temp>120
//    Then any of these two transitions may happen for temp = 130!!
// 5. write the state functions (return fsm_return_t) and have them return
//    FSM_OK. Returning anything else will trigger a callback function that
//    will help you debug what was wrong. (callback not implemented yet)
// 6. Set buffer callback functions in a lfsm_buf_callbacks_t
// 7. Use lfsm_init() to create a state machine context with the arrays from 2.
//    and 3., callbacks from 6. and user data. The maximum number of state
//    machine instances is defined in the config file as LFSM_MAX_COUNT.
// 8. Add events using lfsm_add(), then run the state machine using lfsm_run().
// 9. To deinitalize the state machine, use lfsm_deinit().

// these enums are just for convenience / readability
enum events {
    EV_BUTTON_PRESS = 10,
    EV_MEASURE
};

enum states {
    ST_NORMAL = 1,
    ST_ALARM,
    ST_WARN = 4
};

// -- state transitions: condition functions --
int temperature_okay(lfsm_t context);
int temperature_warning(lfsm_t context);
int temperature_critical(lfsm_t context);
// -- state functions
lfsm_return_t measure_temperature(lfsm_t context);
lfsm_return_t green_led_on(lfsm_t context);
lfsm_return_t green_led_off(lfsm_t context);
lfsm_return_t yellow_led_on(lfsm_t context);
lfsm_return_t yellow_led_off(lfsm_t context);
lfsm_return_t red_led_on(lfsm_t context);
lfsm_return_t red_led_blink(lfsm_t context);
lfsm_return_t red_led_off(lfsm_t context);

// -------------------------------------------------------------------------
// - transition table
// -------------------------------------------------------------------------
lfsm_transitions_t transition_table[] = {
    { ST_ALARM  , EV_BUTTON_PRESS , temperature_okay     , ST_NORMAL },
    { ST_NORMAL , EV_MEASURE      , temperature_critical , ST_WARN   },
    { ST_NORMAL , EV_MEASURE      , temperature_warning  , ST_ALARM  },
    { ST_WARN   , EV_MEASURE      , temperature_okay     , ST_NORMAL },
    { ST_WARN   , EV_MEASURE      , temperature_critical , ST_ALARM  },
};
// -------------------------------------------------------------------------
// - State function table
// -------------------------------------------------------------------------
lfsm_state_functions_t state_func_table[] = {
    {ST_NORMAL, green_led_on , measure_temperature, green_led_off  },
    {ST_WARN  , yellow_led_on, measure_temperature, yellow_led_off },
    {ST_ALARM , red_led_on   , measure_temperature, red_led_off    },
};

// -- Transition condition functions ----
int temperature_okay(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature <= WARN_TEMP;
}

int temperature_warning(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    int higher_than_okay = data->temperature > WARN_TEMP;
    int not_critical     = data->temperature <= ALARM_TEMP;
    return (higher_than_okay && not_critical);
}

int temperature_critical(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature > ALARM_TEMP;
}

// --- State functions ---
lfsm_return_t green_led_on(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->green_led = LED_ON;
    return LFSM_OK;
}

lfsm_return_t green_led_off(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->green_led = LED_OFF;
    return LFSM_OK;
}

lfsm_return_t red_led_on(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->red_led = LED_ON;
    return LFSM_OK;
}

lfsm_return_t red_led_off(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->red_led = LED_OFF;
    return LFSM_OK;
}

lfsm_return_t yellow_led_on(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->yellow_led = LED_ON;
    return LFSM_OK;
}

lfsm_return_t yellow_led_off(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->yellow_led = LED_OFF;
    return LFSM_OK;
}

lfsm_return_t measure_temperature(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->temperature++;
    return LFSM_OK;
}

// -------------------------------------------------------------------------
lfsm_buf_callbacks_t buffer_callbacks;
lfsm_t lfsm_handler;

// -------- UNITY SETUP AND TEARDOWN CALLBACKS ----------
void setUp(void) {
    my_data.red_led = LED_OFF;
    my_data.green_led = LED_OFF;
    buf_init_system();
    lfsm_set_lovely_buf_callbacks(&buffer_callbacks);
}

void tearDown(void) {
    //lfsm_deinit(lfsm_handler);
}

void test_init_lfsm() {
    lfsm_handler = lfsm_init(transition_table, state_func_table, buffer_callbacks, my_data);
    TEST_ASSERT_NOT_NULL(lfsm_handler);
    print_transition_table(lfsm_handler);
    print_transition_lookup_table(lfsm_handler);
}

void test_ignore(void) {
    TEST_IGNORE();
}
