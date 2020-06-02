/* --------------------------------------------------------------------------
 * For this unit test we will simulate a temperature supervisor, that will
 * trigger an alarm on overtemperature and has to be reset by a keypress, once
 * the temperature has gone down.
 * -------------------------------------------------------------------------- */

#include "unity.h"
#include "../../src/lovely_fsm.h"
#include "../../lovelyBuffer/buf_buffer.h"

// -------- User data structures -------------------------------------
enum {
    LED_OFF,
    LED_ON
};

typedef struct my_data_t {
    int temperature;
    int red_led;
    int green_led;
} my_data_t;
my_data_t my_data;

#define MAX_TEMP    100

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
    EV_BUTTON_PRESS,
    EV_TEMP_OVER
};

enum states {
    ST_IDLE,
    ST_ALARM
};

int temperature_okay(lfsm_t context);
int temperature_too_high(lfsm_t context);
lfsm_return_t green_led_on(lfsm_t context);
lfsm_return_t green_led_off(lfsm_t context);
lfsm_return_t red_led_on(lfsm_t context);
lfsm_return_t red_led_blink(lfsm_t context);
lfsm_return_t red_led_off(lfsm_t context);

// -------------------------------------------------------------------------
// - transition table
// -------------------------------------------------------------------------
lfsm_transitions_t transition_table[] = {
    { ST_ALARM, EV_BUTTON_PRESS , temperature_too_high, ST_ALARM },
    { ST_IDLE , EV_BUTTON_PRESS , always              , ST_IDLE  },
    { ST_IDLE , EV_TEMP_OVER    , always              , ST_ALARM },
};

// -- Transition condition functions ----
int temperature_okay(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature < MAX_TEMP;
}

int temperature_too_high(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature > MAX_TEMP;
}
// -------------------------------------------------------------------------
// - State function table
// -------------------------------------------------------------------------
lfsm_state_functions_t state_func_table[] = {
    {ST_IDLE  , green_led_on , NULL         , green_led_off },
    {ST_ALARM , red_led_on   , red_led_blink, red_led_off   },
};
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

lfsm_return_t red_led_blink(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->red_led = LED_ON;
    data->red_led = LED_OFF;
    data->red_led = LED_ON;
    data->red_led = LED_OFF;
    data->red_led = LED_ON;
    return LFSM_OK;
}

lfsm_return_t red_led_off(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    data->red_led = LED_OFF;
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
    lfsm_handler = lfsm_init(transition_table, state_func_table, buffer_callbacks, my_data);
    TEST_ASSERT_NOT_NULL(lfsm_handler);
}

void tearDown(void) {
    lfsm_deinit();
}

void test_ignore(void) {
    TEST_IGNORE();
}
