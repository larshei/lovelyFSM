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
    uint8_t generic_entry_run_count;
    uint8_t generic_run_run_count;
    uint8_t generic_exit_run_count;
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

enum events_large_fsm {EV_0, EV_1, EV_2, EV_3, EV_4, EV_5, EV_6, EV_7, EV_8, EV_9};
enum states_large_fsm {ST_0, ST_1, ST_2, ST_3, ST_4, ST_5, ST_6, ST_7, ST_8, ST_9};

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

lfsm_return_t generic_entry(lfsm_t context);
lfsm_return_t generic_run(lfsm_t context);
lfsm_return_t generic_exit(lfsm_t context);

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

lfsm_transitions_t my_transition_table[] = {
    { ST_0  , EV_0 , lfsm_always , ST_0 },
    { ST_0  , EV_1 , lfsm_always , ST_1 },
    { ST_0  , EV_2 , lfsm_always , ST_2 },
    { ST_0  , EV_3 , lfsm_always , ST_3 },
    { ST_0  , EV_4 , lfsm_always , ST_4 },
    { ST_0  , EV_5 , lfsm_always , ST_5 },
    { ST_0  , EV_6 , lfsm_always , ST_6 },
    { ST_0  , EV_7 , lfsm_always , ST_7 },
    { ST_0  , EV_8 , lfsm_always , ST_8 },
    { ST_0  , EV_9 , lfsm_always , ST_9 },
    { ST_1  , EV_0 , lfsm_always , ST_0 },
    { ST_1  , EV_1 , lfsm_always , ST_1 },
    { ST_1  , EV_2 , lfsm_always , ST_2 },
    { ST_1  , EV_3 , lfsm_always , ST_3 },
    { ST_1  , EV_4 , lfsm_always , ST_4 },
    { ST_1  , EV_5 , lfsm_always , ST_5 },
    { ST_1  , EV_6 , lfsm_always , ST_6 },
    { ST_1  , EV_7 , lfsm_always , ST_7 },
    { ST_1  , EV_8 , lfsm_always , ST_8 },
    { ST_1  , EV_9 , lfsm_always , ST_9 },
    { ST_2  , EV_0 , lfsm_always , ST_0 },
    { ST_2  , EV_1 , lfsm_always , ST_1 },
    { ST_2  , EV_2 , lfsm_always , ST_2 },
    { ST_2  , EV_4 , lfsm_always , ST_4 },
    { ST_2  , EV_5 , lfsm_always , ST_5 },
    { ST_2  , EV_6 , lfsm_always , ST_6 },
    { ST_2  , EV_7 , lfsm_always , ST_7 },
    { ST_2  , EV_8 , lfsm_always , ST_8 },
    { ST_2  , EV_9 , lfsm_always , ST_9 },
    { ST_3  , EV_0 , lfsm_always , ST_0 },
    { ST_3  , EV_1 , lfsm_always , ST_1 },
    { ST_3  , EV_2 , lfsm_always , ST_2 },
    { ST_3  , EV_3 , lfsm_always , ST_3 },
    { ST_3  , EV_4 , lfsm_always , ST_4 },
    { ST_3  , EV_5 , lfsm_always , ST_5 },
    { ST_3  , EV_6 , lfsm_always , ST_6 },
    { ST_3  , EV_7 , lfsm_always , ST_7 },
    { ST_3  , EV_8 , lfsm_always , ST_8 },
    { ST_3  , EV_9 , lfsm_always , ST_9 },
    { ST_4  , EV_1 , lfsm_always , ST_1 },
    { ST_4  , EV_3 , lfsm_always , ST_3 },
    { ST_4  , EV_4 , lfsm_always , ST_4 },
    { ST_4  , EV_5 , lfsm_always , ST_5 },
    { ST_4  , EV_6 , lfsm_always , ST_6 },
    { ST_4  , EV_7 , lfsm_always , ST_7 },
    { ST_4  , EV_9 , lfsm_always , ST_9 },
    { ST_5  , EV_0 , lfsm_always , ST_0 },
    { ST_5  , EV_1 , lfsm_always , ST_1 },
    { ST_5  , EV_2 , lfsm_always , ST_2 },
    { ST_5  , EV_3 , lfsm_always , ST_3 },
    { ST_5  , EV_4 , lfsm_always , ST_4 },
    { ST_5  , EV_5 , lfsm_always , ST_5 },
    { ST_5  , EV_6 , lfsm_always , ST_6 },
    { ST_5  , EV_7 , lfsm_always , ST_7 },
    { ST_5  , EV_8 , lfsm_always , ST_8 },
    { ST_5  , EV_9 , lfsm_always , ST_9 },
    { ST_6  , EV_0 , lfsm_always , ST_0 },
    { ST_6  , EV_1 , lfsm_always , ST_1 },
    { ST_6  , EV_2 , lfsm_always , ST_2 },
    { ST_6  , EV_3 , lfsm_always , ST_3 },
    { ST_6  , EV_5 , lfsm_always , ST_5 },
    { ST_6  , EV_6 , lfsm_always , ST_6 },
    { ST_6  , EV_7 , lfsm_always , ST_7 },
    { ST_6  , EV_8 , lfsm_always , ST_8 },
    { ST_7  , EV_0 , lfsm_always , ST_0 },
    { ST_7  , EV_1 , lfsm_always , ST_1 },
    { ST_7  , EV_2 , lfsm_always , ST_2 },
    { ST_7  , EV_3 , lfsm_always , ST_3 },
    { ST_7  , EV_4 , lfsm_always , ST_4 },
    { ST_7  , EV_6 , lfsm_always , ST_6 },
    { ST_7  , EV_7 , lfsm_always , ST_7 },
    { ST_7  , EV_8 , lfsm_always , ST_8 },
    { ST_7  , EV_9 , lfsm_always , ST_9 },
    { ST_8  , EV_0 , lfsm_always , ST_0 },
    { ST_8  , EV_1 , lfsm_always , ST_1 },
    { ST_8  , EV_2 , lfsm_always , ST_2 },
    { ST_8  , EV_3 , lfsm_always , ST_3 },
    { ST_8  , EV_4 , lfsm_always , ST_4 },
    { ST_8  , EV_5 , lfsm_always , ST_5 },
    { ST_8  , EV_6 , lfsm_always , ST_6 },
    { ST_8  , EV_7 , lfsm_always , ST_7 },
    { ST_8  , EV_8 , lfsm_always , ST_8 },
    { ST_8  , EV_9 , lfsm_always , ST_9 },
    { ST_9  , EV_0 , lfsm_always , ST_0 },
    { ST_9  , EV_1 , lfsm_always , ST_1 },
    { ST_9  , EV_2 , lfsm_always , ST_2 },
    { ST_9  , EV_3 , lfsm_always , ST_3 },
    { ST_9  , EV_4 , lfsm_always , ST_4 },
    { ST_9  , EV_5 , lfsm_always , ST_5 },
    { ST_9  , EV_6 , lfsm_always , ST_6 },
    { ST_9  , EV_7 , lfsm_always , ST_7 },
    { ST_9  , EV_8 , lfsm_always , ST_8 },
    { ST_9  , EV_9 , lfsm_always , ST_9 },
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

lfsm_state_functions_t my_state_func_table[] = {
    { ST_0  , generic_entry , generic_run , generic_exit },
    { ST_1  , generic_entry , generic_run , generic_exit },
    { ST_2  , generic_entry , generic_run , generic_exit },
    { ST_3  , generic_entry , generic_run , generic_exit },
    { ST_4  , generic_entry , generic_run , generic_exit },
    { ST_5  , generic_entry , generic_run , generic_exit },
    { ST_6  , generic_entry , generic_run , generic_exit },
    { ST_7  , generic_entry , generic_run , generic_exit },
    { ST_8  , generic_entry , generic_run , generic_exit },
    { ST_9  , generic_entry , generic_run , generic_exit },
};

// -- Transition condition functions ----
int temperature_okay(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature <= WARN_TEMP;
}

int temperature_warning(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    int higher_than_okay = data->temperature >= WARN_TEMP;
    int not_critical     = data->temperature < ALARM_TEMP;
    return (higher_than_okay && not_critical);
}

int temperature_critical(lfsm_t context) {
    my_data_t* data = (my_data_t*)lfsm_user_data(context);
    return data->temperature >= ALARM_TEMP;
}

// --- State functions ---

lfsm_return_t alarm_entry(lfsm_t context){
    // printf("Running alarm_entry\n");
    my_data.alarm_entry_run_count++;
    return LFSM_OK;
}
lfsm_return_t alarm_run(lfsm_t context){
    // printf("Running alarm_run\n");
    my_data.alarm_run_run_count++;
    return LFSM_OK;
}
lfsm_return_t alarm_exit(lfsm_t context){
    // printf("Running alarm_exit\n");
    my_data.alarm_exit_run_count++;
    return LFSM_OK;
}
lfsm_return_t warn_entry(lfsm_t context){
    // printf("Running warn_entry\n");
    my_data.warn_entry_run_count++;
    return LFSM_OK;
}
lfsm_return_t warn_run(lfsm_t context){
    // printf("Running warn_run\n");
    my_data.warn_run_run_count++;
    return LFSM_OK;
}
lfsm_return_t warn_exit(lfsm_t context){
    // printf("Running warn_exit\n");
    my_data.warn_exit_run_count++;
    return LFSM_OK;
}
lfsm_return_t normal_entry(lfsm_t context){
    // printf("Running normal_entry\n");
    my_data.normal_entry_run_count++;
    return LFSM_OK;
}
lfsm_return_t normal_run(lfsm_t context){
    // printf("Running normal_run\n");
    my_data.normal_run_run_count++;
    return LFSM_OK;
}
lfsm_return_t normal_exit(lfsm_t context){
    // printf("Running normal_exit\n");
    my_data.normal_exit_run_count++;
    return LFSM_OK;
}

lfsm_return_t generic_entry(lfsm_t context){
    my_data.generic_entry_run_count++;
    return LFSM_OK;
}
lfsm_return_t generic_run(lfsm_t context){
    my_data.generic_run_run_count++;
    return LFSM_OK;
}
lfsm_return_t generic_exit(lfsm_t context){
    my_data.generic_exit_run_count++;
    return LFSM_OK;
}


// -------------------------------------------------------------------------
lfsm_buf_callbacks_t buffer_callbacks;
lfsm_t lfsm_handler;

// -------- UNITY SETUP AND TEARDOWN CALLBACKS ----------
void setUp(void) {
    memset((char*)&my_data, 0, sizeof(my_data_t));
    buf_init_system();
    lfsm_set_lovely_buf_callbacks(&buffer_callbacks);
    lfsm_handler = lfsm_init(transition_table, state_func_table, buffer_callbacks, &my_data, ST_NORMAL);
    TEST_ASSERT_NOT_NULL(lfsm_handler);
}

void tearDown(void) {
    lfsm_deinit(lfsm_handler);
}

void test_init_lfsm() {
    TEST_ASSERT_NOT_NULL(lfsm_handler);
    print_transition_table(lfsm_handler);
    print_state_function_table(lfsm_handler);
}

void test_set_get_state() {
    lfsm_set_state(lfsm_handler, ST_NORMAL);
    TEST_ASSERT_EQUAL(ST_NORMAL, lfsm_get_state(lfsm_handler));
    lfsm_set_state(lfsm_handler, ST_ALARM);
    TEST_ASSERT_EQUAL(ST_ALARM, lfsm_get_state(lfsm_handler));
    lfsm_set_state(lfsm_handler, ST_WARN);
    TEST_ASSERT_EQUAL(ST_WARN, lfsm_get_state(lfsm_handler));
}


void test_no_event_queued_at_start( void ) {
    TEST_ASSERT_EQUAL(1, lfsm_no_event_queued(lfsm_handler));
}


void test_run_with_no_events( void ) {
    lfsm_return_t ret = lfsm_run(lfsm_handler);
    TEST_ASSERT_EQUAL(1, lfsm_no_event_queued(lfsm_handler));
    TEST_ASSERT_EQUAL(LFSM_NOP, ret);
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


void test_run_transitions( void ) {
    uint8_t event;
    lfsm_return_t ret;


    // printf("\n------------------------\n");
    // set state, add event, set temperature
    lfsm_set_state(lfsm_handler, ST_NORMAL);
    fsm_add_event(lfsm_handler, EV_MEASURE);
    my_data.temperature = WARN_TEMP - 5; // should stay on normal!
    ret = lfsm_run(lfsm_handler);
    TEST_ASSERT_EQUAL(LFSM_NOP, ret);
    TEST_ASSERT_EQUAL(ST_NORMAL, lfsm_get_state(lfsm_handler));
    TEST_ASSERT_EQUAL(1, my_data.normal_run_run_count);
    TEST_ASSERT_EQUAL(1, my_data.normal_entry_run_count);
    // all others 0
    my_data.normal_run_run_count = 0;
    my_data.normal_entry_run_count = 0;
    my_data.temperature = 0;
    TEST_ASSERT_EACH_EQUAL_UINT8(0, &my_data, sizeof(my_data_t));

    lfsm_set_state(lfsm_handler, ST_NORMAL);
    fsm_add_event(lfsm_handler, EV_MEASURE);
    my_data.temperature = WARN_TEMP + 5; // should go to warn!
    TEST_ASSERT_EQUAL(ST_NORMAL, lfsm_get_state(lfsm_handler));
    // printf("run to transition from NORMAL to WARN...\n");
    lfsm_run(lfsm_handler);
    // printf("--------------\n");
    TEST_ASSERT_EQUAL(ST_WARN, lfsm_get_state(lfsm_handler));
    TEST_ASSERT_EQUAL(1, my_data.warn_entry_run_count);
    TEST_ASSERT_EQUAL(1, my_data.warn_run_run_count);
    TEST_ASSERT_EQUAL(1, my_data.normal_exit_run_count);
}

void test_run_non_existing_state_event_combo(void) {
    lfsm_set_state(lfsm_handler, ST_NORMAL);
    fsm_add_event(lfsm_handler, EV_BUTTON_PRESS);
    lfsm_run(lfsm_handler);
}

void test_create_large_second_fsm_instance(void) {
    lfsm_handler = lfsm_init(my_transition_table, my_state_func_table, buffer_callbacks, &my_data, ST_0);
    TEST_ASSERT_NOT_NULL(lfsm_handler);
}

void test_execute_all_transitions_once_should_not_crash(void) {
    int transitions_without_state_change = 0;
    int transition_count = sizeof(my_transition_table) / sizeof(lfsm_transitions_t);

    test_create_large_second_fsm_instance();
    for (int state = 0 ; state < 10 ; state++) {
    for (int event = 0 ; event < 10 ; event++){
        lfsm_set_state(lfsm_handler, state);
        fsm_add_event(lfsm_handler, event);
        lfsm_run(lfsm_handler);
    }
    }

    // start at entry 1, cause entry 0 will always go from INVALID to anything
    lfsm_transitions_t* transition = my_transition_table;
    transition++;
    for (int i = 1 ; i < transition_count ; i++) {
        if (transition->current_state == transition->event) {
            transitions_without_state_change++;
        }
        transition++;
    }
    TEST_ASSERT_EQUAL(transition_count - transitions_without_state_change,
                        my_data.generic_entry_run_count);
}
