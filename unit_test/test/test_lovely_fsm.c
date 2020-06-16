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
    uint8_t alarm_entry_run_count;
    uint8_t alarm_run_run_count;
    uint8_t alarm_exit_run_count;
    uint8_t warn_entry_run_count;
    uint8_t warn_run_run_count;
    uint8_t warn_exit_run_count;
    uint8_t normal_entry_run_count;
    uint8_t normal_run_run_count;
    uint8_t normal_exit_run_count;
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
lfsm_return_t alarm_entry(lfsm_t context);
lfsm_return_t alarm_run(lfsm_t context);
lfsm_return_t alarm_exit(lfsm_t context);
lfsm_return_t warn_entry(lfsm_t context);
lfsm_return_t warn_run(lfsm_t context);
lfsm_return_t warn_exit(lfsm_t context);
lfsm_return_t normal_entry(lfsm_t context);
lfsm_return_t normal_run(lfsm_t context);
lfsm_return_t normal_exit(lfsm_t context);


// -------------------------------------------------------------------------
// - transition table
// -------------------------------------------------------------------------
lfsm_transitions_t transition_table[] = {
    // STATE      EVENT             CONDITION              TRANSITION TO
    { ST_ALARM  , EV_BUTTON_PRESS , temperature_okay     , ST_NORMAL },
    { ST_NORMAL , EV_MEASURE      , temperature_warning  , ST_WARN   },
    { ST_NORMAL , EV_MEASURE      , temperature_critical , ST_ALARM  },
    { ST_WARN   , EV_MEASURE      , temperature_okay     , ST_NORMAL },
    { ST_WARN   , EV_MEASURE      , temperature_critical , ST_ALARM  },
};
// -------------------------------------------------------------------------
// - State function table
// -------------------------------------------------------------------------
lfsm_state_functions_t state_func_table[] = {
    // STATE     ON_ENTRY()     ON_RUN()    ON_EXIT()
    {ST_NORMAL , normal_entry , normal_run, normal_exit },
    {ST_WARN   , warn_entry   , warn_run  , warn_exit   },
    {ST_ALARM  , alarm_entry  , alarm_run , alarm_exit  },
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

lfsm_return_t alarm_entry(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.alarm_entry_run_count++;
}
lfsm_return_t alarm_run(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.alarm_run_run_count++;
}
lfsm_return_t alarm_exit(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.alarm_exit_run_count++;
}
lfsm_return_t warn_entry(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.warn_entry_run_count++;
}
lfsm_return_t warn_run(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.warn_run_run_count++;
}
lfsm_return_t warn_exit(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.warn_exit_run_count++;
}
lfsm_return_t normal_entry(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.normal_entry_run_count++;
}
lfsm_return_t normal_run(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.normal_run_run_count++;
}
lfsm_return_t normal_exit(lfsm_t context){
    printf("Running %s\n", __func__);
    my_data.normal_exit_run_count++;
}


// -------------------------------------------------------------------------
lfsm_buf_callbacks_t buffer_callbacks;
lfsm_t lfsm_handler;

// -------- UNITY SETUP AND TEARDOWN CALLBACKS ----------
void setUp(void) {
    memset((char*)&my_data, 0, sizeof(my_data_t));
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
    print_state_function_table(lfsm_handler);
    print_state_function_lookup_table(lfsm_handler);
}

void test_set_get_state() {
    lfsm_set_state(lfsm_handler, ST_NORMAL);
    TEST_ASSERT_EQUAL(ST_NORMAL, lfsm_get_state(lfsm_handler));
    lfsm_set_state(lfsm_handler, ST_ALARM);
    TEST_ASSERT_EQUAL(ST_ALARM, lfsm_get_state(lfsm_handler));
    lfsm_set_state(lfsm_handler, ST_WARN);
    TEST_ASSERT_EQUAL(ST_WARN, lfsm_get_state(lfsm_handler));
}



// (lookup table points to first element of similar state/event transitions in
// a sorted transition list.
// All combinations of states/events are checked. For each possible
// combination, we manually run through the (sorted) transition table, until we
// find the correct state/event combo. This is the locaton the lookup table
// should point to 
void test_get_transition_address_from_lookup( void ) {
    lfsm_transitions_t* transition_from_lookup;
    lfsm_transitions_t* transition_temporary;
    lfsm_transitions_t* transition_for_loop_runner;
    uint8_t emin = lfsm_get_event_min(lfsm_handler);
    uint8_t emax = lfsm_get_event_max(lfsm_handler);
    uint8_t smin = lfsm_get_state_min(lfsm_handler);
    uint8_t smax = lfsm_get_state_max(lfsm_handler);
    int transition_count = lfsm_get_transition_count(lfsm_handler);

    for (int state = smin ; state <= smax ; state++) {
        lfsm_set_state(lfsm_handler, state);

        for (int event = emin ; event <= emax ; event++) {
            transition_temporary = NULL;
            transition_for_loop_runner = lfsm_get_transition_table(lfsm_handler);

            for (int transition = 0 ; transition < transition_count ; transition++ ) {
                if (transition_for_loop_runner->current_state == state) {
                if (transition_for_loop_runner->event == event) {
                    transition_temporary = transition_for_loop_runner;
                    break; // leave loop -> found first transition for state/event
                }
                }
                transition_for_loop_runner++;
            }

            transition_from_lookup = lfsm_get_transition_from_lookup(lfsm_handler, event);
            TEST_ASSERT_EQUAL(transition_temporary, transition_from_lookup);
        }
    }
    lfsm_set_state(lfsm_handler, ST_NORMAL);
}

void test_no_event_queued_at_start( void ) {
    TEST_ASSERT_EQUAL(1, lfsm_no_event_queued(lfsm_handler));
}

void test_add_event_to_buffer_and_read( void ) {
    uint8_t event;

    for (int i = 0 ; i < LFSM_EV_QUEUE_SIZE ; i++) {
        event = lfsm_read_event_queue_element(lfsm_handler, i);
        TEST_ASSERT_EQUAL(0, event);
    }

    fsm_add_event(lfsm_handler, EV_MEASURE);
    event = lfsm_read_event_queue_element(lfsm_handler, 0);
    TEST_ASSERT_EQUAL(EV_MEASURE, event);
    event = lfsm_read_event(lfsm_handler);
    TEST_ASSERT_EQUAL(EV_MEASURE, event);

    fsm_add_event(lfsm_handler, EV_BUTTON_PRESS);
    event = lfsm_read_event_queue_element(lfsm_handler, 1);
    TEST_ASSERT_EQUAL(EV_BUTTON_PRESS, event);
    event = lfsm_read_event(lfsm_handler);
    TEST_ASSERT_EQUAL(EV_BUTTON_PRESS, event);
}

void test_run_with_no_events( void ) {
    lfsm_return_t ret = lfsm_run(lfsm_handler);
    TEST_ASSERT_EQUAL(1, lfsm_no_event_queued(lfsm_handler));
    TEST_ASSERT_EQUAL(LFSM_NOP, ret);
}

void test_run_transitions( void ) {
    uint8_t event;
    lfsm_return_t ret;


    printf("\n------------------------\n");
    // set state, add event, set temperature
    lfsm_set_state(lfsm_handler, ST_NORMAL);
    fsm_add_event(lfsm_handler, EV_MEASURE);
    my_data.temperature = WARN_TEMP - 5; // should stay on normal!
    ret = lfsm_run(lfsm_handler);
    TEST_ASSERT_EQUAL(LFSM_OK, ret);
    TEST_ASSERT_EQUAL(ST_NORMAL, lfsm_get_state(lfsm_handler));
    TEST_ASSERT_EQUAL(1, my_data.normal_run_run_count);
    // all others 0
    my_data.normal_run_run_count = 0;
    my_data.temperature = 0;
    TEST_ASSERT_EACH_EQUAL_UINT8(0, &my_data, sizeof(my_data_t));

    lfsm_set_state(lfsm_handler, ST_NORMAL);
    fsm_add_event(lfsm_handler, EV_MEASURE);
    my_data.temperature = WARN_TEMP + 5; // should go to warn!
    lfsm_run(lfsm_handler);
    TEST_ASSERT_EQUAL(ST_WARN, lfsm_get_state(lfsm_handler));
}
