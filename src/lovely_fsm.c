#include "lovely_fsm.h"

/* -----------------------------------------------------------------------------
 * Managed internally, user needs lfsm_context_t (pointer) only
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t {
    uint8_t is_active;
    uint8_t state_count;
    uint8_t transition_count;
    uint8_t current_state;
    uint8_t previous_step_state;
    uint8_t event_queue_buffer[LFSM_EV_QUEUE_SIZE];
    buffer_handle_type buffer_handle;
    void*   user_data;
    lfsm_transitions_t*     transition_table;
    lfsm_state_functions_t* functions_table;
} lfsm_context_t;

typedef struct lfsm_system_t {
    lfsm_context_t contexts[LFSM_MAX_COUNT];
} lfsm_system_t;

lfsm_system_t system;

// public functions
lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);

// private functions
lfsm_t lfsm_get_unused_context();
lfsm_t lfsm_init_func();
void lfsm_sort_list(lfsm_t context);

void presort_list_by_state(lfsm_t context);
void swap_elements(lfsm_transitions_t* items, int index_first, int index_second );


// ----------------------------------------------------------------------------
lfsm_t lfsm_get_unused_context() {
    int index = 0;
    for (int index = 0 ; index < LFSM_MAX_COUNT ; index++) {
        if (!(system.contexts[index].is_active)) return &system.contexts[index];
    }
    return NULL;
}

lfsm_return_t lfsm_initialize_buffers(fsm_t fsm) {
#if (USE_BUFFER_C_LIB)
    buf_data_info_t data_info;
    data_info.array = fsm->event_queue_buffer;
    data_info.element_count = LFSM_EV_QUEUE_SIZE;
    data_info.element_size = sizeof(int);
    fsm->buffer_handle = get_buffer(&data_info);
#else
    fsm->buffer_handle = get_buffer(fsm->event_queue_buffer, LFSM_EV_QUEUE_SIZE, sizeof(int));
#endif
}

lfsm_t lfsm_init_func(void* user_data) {
    lfsm_t new_fsm = lfsm_get_unused_context();

    if (new_fsm) {
        lfsm_initialize_buffers(new_fsm);
        lfsm_sort_list(new_fsm);
        // create lookups
        new_fsm->user_data = user_data;
    }
    return new_fsm;
}

// ----------------------------------------------------------------------------

lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event) {
    // add event to context buffer
}

void swap_elements(lfsm_transitions_t* items, int index_first, int index_second ) {
    lfsm_transitions_t temp_item;
    temp_item           = items[index_first];
    items[index_first]  = items[index_second];
    items[index_second] = temp_item;
}

void lfsm_sort_list(lfsm_t context) {
    lfsm_transitions_t* sorter;
    int swap_for_state, swap_for_event, same_state;
    int list_length = context->transition_count;

    for (int unsorted = list_length - 1; unsorted > 0 ; unsorted--) {
        sorter = context->transition_table;
        for (int i = 0; i < unsorted; i++) {
            swap_for_state = sorter->current_state >  (sorter+1)->current_state;
            same_state     = sorter->current_state == (sorter+1)->current_state;
            swap_for_event = sorter->event         >  (sorter+1)->event;

            if (swap_for_state || (same_state && swap_for_event) ) {
                swap_elements(context->transition_table, i, i+1);
            }
            sorter++;
        }
    }
}
