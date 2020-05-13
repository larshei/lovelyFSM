#include "lovely_fsm.h"
#include "buf_buffer.h"

/* -----------------------------------------------------------------------------
 * Managed internally, user needs lfsm_context_t (pointer) only
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t {
    void* user_data_in;
    void* user_data_iout;
    uint8_t current_state;
    uint8_t previous_step_state;
    uint8_t event_queue_buffer[LFSM_EV_QUEUE_SIZE];
    lfsm_transitions_t      * transition_table;
    lfsm_state_functions_t  * functions_table;
} lfsm_context_t;


lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event) {

}


