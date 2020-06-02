#include "lovely_fsm.h"
#include <stdlib.h>

#ifdef TEST
#include "lovely_fsm_debug.c"
#endif

/* -----------------------------------------------------------------------------
 * Managed internally, user needs lfsm_context_t (pointer) only
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t {
    uint8_t is_active;
    uint8_t state_number_min;
    uint8_t state_number_max;
    uint8_t event_number_min;
    uint8_t event_number_max;
    uint8_t transition_count;
    uint8_t state_func_count;
    uint8_t current_state;
    uint8_t previous_step_state;
    uint8_t event_queue_buffer[LFSM_EV_QUEUE_SIZE];
    lfsm_buf_callbacks_t buf_func;
    buffer_handle_type buffer_handle;
    void*   user_data;
    lfsm_transitions_t*     transition_table;
    lfsm_state_functions_t* functions_table;
    lfsm_transitions_t*     lookup_table;
} lfsm_context_t;

typedef struct lfsm_system_t {
    lfsm_context_t contexts[LFSM_MAX_COUNT];
} lfsm_system_t;

lfsm_system_t lfsm_system;

// public functions
lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);

// private functions
lfsm_t lfsm_get_unused_context();
lfsm_t lfsm_init_func();
void lfsm_bubble_sort_list(lfsm_t context);
void lfsm_find_state_event_min_max(lfsm_t context);
lfsm_return_t lfsm_create_lookup_table(lfsm_t context);
void lfsm_fill_lookup_table(lfsm_t context);

// ----------------------------------------------------------------------------
lfsm_t lfsm_get_unused_context() {
    int index = 0;
    for (int index = 0 ; index < LFSM_MAX_COUNT ; index++) {
        if (!(lfsm_system.contexts[index].is_active)) return &lfsm_system.contexts[index];
    }
    return NULL;
}

lfsm_return_t lfsm_initialize_buffers(lfsm_t fsm) {
#if (USE_LOVELY_BUFFER)
    buf_data_info_t data_info;
    data_info.array = fsm->event_queue_buffer;
    data_info.element_count = LFSM_EV_QUEUE_SIZE;
    data_info.element_size = sizeof(int);
    fsm->buffer_handle = fsm->buf_func.init(&data_info);
#else
    fsm->buffer_handle = get_buffer(fsm->event_queue_buffer, LFSM_EV_QUEUE_SIZE, sizeof(int));
#endif
    if (fsm->buffer_handle == NULL) return LFSM_ERROR;
    return LFSM_OK;
}

lfsm_t lfsm_init_func(lfsm_transitions_t* transitions, \
                        int trans_count,\
                        lfsm_state_functions_t* states,\
                        int state_count,\
                        void* user_data) 
{
    lfsm_t new_fsm = lfsm_get_unused_context();
    if (new_fsm) {
        new_fsm->functions_table = states;
        new_fsm->state_func_count = state_count;
        new_fsm->transition_table = transitions;
        new_fsm->transition_count = trans_count;
        // if (lfsm_initialize_buffers(new_fsm) == LFSM_OK) {
            print_transition_table(new_fsm);
            lfsm_bubble_sort_list(new_fsm);
            print_transition_table(new_fsm);
            lfsm_find_state_event_min_max(new_fsm);
            // if (lfsm_create_lookup_table(new_fsm) == LFSM_OK) {
            //     lfsm_fill_lookup_table(new_fsm);
            //     new_fsm->user_data = user_data;
            //     return new_fsm;
            // }
        // }
    }
    return NULL;
}

lfsm_return_t lfsm_deinit(lfsm_t context) {
    memset(context, 0, sizeof(lfsm_context_t));
}

// ----------------------------------------------------------------------------

lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event) {
    uint8_t error = context->buf_func.add(event);
    if (error) return LFSM_ERROR;
    return LFSM_OK;
}

void swap_elements(lfsm_transitions_t* items, int index_first, int index_second ) {
    lfsm_transitions_t temp_item;
    temp_item           = items[index_first];
    items[index_first]  = items[index_second];
    items[index_second] = temp_item;
}

void lfsm_bubble_sort_list(lfsm_t context) {
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

uint8_t max(uint8_t a, uint8_t b) {
    if (a > b) return a;
    return b;
}

uint8_t min(uint8_t a, uint8_t b) {
    if (a < b) return a;
    return b;
}

void lfsm_find_state_event_min_max(lfsm_t context) {
    int list_length = context->transition_count;
    lfsm_transitions_t* transition = context->transition_table;
    uint8_t max_state = 0;
    uint8_t min_state  = 255;
    uint8_t max_event = 0;
    uint8_t min_event  = 255;

    for (int i = 0; i < list_length ; i++, transition++) {
        min_event = min(min_event, transition->event);
        max_event = max(max_event, transition->event);

        min_state = min(max_state, min(transition->current_state, transition->next_state));
        max_state = max(max_state, max(transition->current_state, transition->next_state));
    }
    context->state_number_min = min_state;
    context->state_number_max = max_state;
    context->event_number_min = min_event;
    context->event_number_max = max_event;
}

lfsm_return_t lfsm_create_lookup_table(lfsm_t context) {
    uint8_t range_state_numbers = context->state_number_max - context->state_number_min;
    uint8_t range_event_numbers = context->event_number_max - context->event_number_min;
    
    uint32_t max_lookup_elements = range_state_numbers * range_event_numbers;
    context->lookup_table = malloc(max_lookup_elements * sizeof(lfsm_transitions_t*));
    if (context->lookup_table == NULL) return LFSM_ERROR;
    return LFSM_OK;
}

void lfsm_fill_lookup_table(lfsm_t context) {
    int list_length = context->transition_count;
    uint8_t state_offset = context->state_number_min;
    uint8_t state_diff   = context->state_number_max - state_offset;
    uint8_t event_offset = context->event_number_min;
    lfsm_transitions_t* transition = context->transition_table;
    lfsm_transitions_t* lookup_table = context->lookup_table;

    uint8_t last_state = transition->current_state;
    uint8_t last_event = transition->event;
    uint8_t state, event;

    for (int i = 0; i < list_length ; i++, transition++) {
        state = transition->current_state;
        event = transition->event;

        if (last_state != state || last_event != event) {
            // in memory: st0 ev0, st0 ev1, st0 ev2, ..., st1 ev0, st1 ev1, ...
            int address_offset = (state - state_offset) * state_diff + event - event_offset;
            transition = ((lfsm_transitions_t*)((int)lookup_table + address_offset));
            last_event = transition->current_state;
            last_state = transition->event;
        }
    }
}

int lfsm_always() {
    return 1;
}

void* lfsm_user_data(lfsm_t context) {
    return context->user_data;
}
#define TEST
#ifdef TEST
lfsm_transitions_t* lfsm_get_transition_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->transition_table;
}
int lfsm_get_transition_count(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->transition_count;
}
lfsm_state_functions_t* lfsm_get_state_function_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->functions_table;
}
int lfsm_get_state_function_count(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->state_func_count;
}
#endif
