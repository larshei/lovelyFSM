#include "lovely_fsm.h"
#include <stdlib.h>
#include <string.h>

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
    lfsm_transitions_t*      transition_table;
    lfsm_state_functions_t*  functions_table;
    lfsm_transitions_t**     transition_lookup_table;
    lfsm_state_functions_t** function_lookup_table;
} lfsm_context_t;

typedef struct lfsm_system_t {
    lfsm_context_t contexts[LFSM_MAX_COUNT];
} lfsm_system_t;
lfsm_system_t lfsm_system;

typedef struct lfsm_lookup_element_t {
    lfsm_transitions_t* transition;
    lfsm_state_functions_t* functions;
} lfsm_lookup_element_t;

// public functions
lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);

// private functions
lfsm_t lfsm_get_unused_context();
lfsm_t lfsm_init_func();
lfsm_return_t lfsm_set_context_buf_callbacks(lfsm_t new_fsm, lfsm_buf_callbacks_t buffer_callbacks);

void lfsm_bubble_sort_list(lfsm_t context);
void lfsm_find_state_event_min_max(lfsm_t context);
lfsm_return_t lfsm_alloc_lookup_table(lfsm_t context);
lfsm_return_t lfsm_fill_transition_lookup_table(lfsm_t context);
lfsm_return_t lfsm_fill_state_function_lookup_table(lfsm_t context);

// ----------------------------------------------------------------------------
lfsm_t lfsm_get_unused_context() {
    int index = 0;
    for (int index = 0 ; index < LFSM_MAX_COUNT ; index++) {
        if (!(lfsm_system.contexts[index].is_active)) return &lfsm_system.contexts[index];
    }
    return NULL;
}

#if (USE_LOVELY_BUFFER)
lfsm_return_t lfsm_set_lovely_buf_callbacks(lfsm_buf_callbacks_t* callbacks) {
    callbacks->system_init = buf_init_system;
    callbacks->init = buf_claim_and_init_buffer;
    callbacks->is_empty = buf_is_empty;
    callbacks->is_full = buf_is_full;
    callbacks->add  = buf_add_element;
    callbacks->read = buf_read_element;
    return LFSM_OK;
}
#endif

lfsm_return_t lfsm_initialize_buffers(lfsm_t fsm) {
#if (USE_LOVELY_BUFFER)
    buf_data_info_t data_info;
    data_info.array = fsm->event_queue_buffer;
    data_info.element_count = LFSM_EV_QUEUE_SIZE;
    data_info.element_size = sizeof(int);
    if (fsm->buf_func.init != NULL) {
        fsm->buffer_handle = fsm->buf_func.init(&data_info);
    }
#else
    fsm->buffer_handle = fsm->buf_func.init(fsm->event_queue_buffer, LFSM_EV_QUEUE_SIZE, sizeof(int));
#endif
    if (fsm->buffer_handle == NULL) return LFSM_ERROR;
    return LFSM_OK;
}


lfsm_return_t lfsm_set_context_buf_callbacks(lfsm_t context, lfsm_buf_callbacks_t buffer_callbacks){
    lfsm_context_t* fsm = (lfsm_context_t*)context;
    context->buf_func = buffer_callbacks;
    // todo: add checks for NULL?!
    return LFSM_OK;
}


lfsm_t lfsm_init_func(lfsm_transitions_t* transitions, \
                        int trans_count,\
                        lfsm_state_functions_t* states,\
                        int state_count,\
                        lfsm_buf_callbacks_t buffer_callbacks, \
                        void* user_data)
{
    lfsm_t new_fsm = lfsm_get_unused_context();
    if (new_fsm) {
        new_fsm->functions_table = states;
        new_fsm->state_func_count = state_count;
        new_fsm->transition_table = transitions;
        new_fsm->transition_count = trans_count;
        // lfsm_set_context_buf_callbacks(new_fsm, buffer_callbacks);
        // if (lfsm_initialize_buffers(new_fsm) == LFSM_OK) {
            print_transition_table(new_fsm);
            lfsm_bubble_sort_list(new_fsm);
            print_transition_table(new_fsm);
            lfsm_find_state_event_min_max(new_fsm);
            if (lfsm_alloc_lookup_table(new_fsm) == LFSM_OK) {
                lfsm_fill_transition_lookup_table(new_fsm);
                lfsm_fill_state_function_lookup_table(new_fsm);
                print_transition_lookup_table(new_fsm);
            //     new_fsm->user_data = user_data;
            //     return new_fsm;
            }
        // }
    }
    return NULL;
}

lfsm_return_t lfsm_deinit(lfsm_t context) {
    lfsm_context_t* fsm = (lfsm_context_t*)context;
    free(fsm->transition_lookup_table);
    free(fsm->function_lookup_table);
    memset((unsigned char*)context, 0, sizeof(lfsm_context_t));
    return LFSM_OK;
}

// ----------------------------------------------------------------------------
lfsm_return_t fsm_add_event(lfsm_t context, DATA_TYPE event) {
    uint8_t error = context->buf_func.add(context->buffer_handle, event);
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

        min_state = min(min_state, min(transition->current_state, transition->next_state));
        max_state = max(max_state, max(transition->current_state, transition->next_state));
    }
    context->state_number_min = min_state;
    context->state_number_max = max_state;
    context->event_number_min = min_event;
    context->event_number_max = max_event;
}

lfsm_return_t lfsm_alloc_lookup_table(lfsm_t context) {
    uint8_t range_state_numbers = context->state_number_max - context->state_number_min + 1;
    uint8_t range_event_numbers = context->event_number_max - context->event_number_min + 1;
    
    uint32_t max_lookup_elements = range_state_numbers * range_event_numbers;
    printf("reserving memory for %d transition lookup elements...\n", max_lookup_elements);
    context->transition_lookup_table = malloc(max_lookup_elements * sizeof(lfsm_transitions_t*));
    if (context->transition_lookup_table == NULL) {
        return LFSM_ERROR;
    }
    printf("OK, reserved @%u!\n", (int)context->transition_lookup_table);
    printf("reserving memory for %d function lookup elements...\n", range_state_numbers);
    context->function_lookup_table = malloc(range_state_numbers * sizeof(lfsm_state_functions_t*));
    if (context->function_lookup_table == NULL) {
        free(context->transition_lookup_table);
        return LFSM_ERROR;
    }
    printf("OK, reserved @%u!\n", (int)context->function_lookup_table);
    return LFSM_OK;
}

lfsm_return_t lfsm_fill_transition_lookup_table(lfsm_t context) {
    int transition_count = lfsm_get_transition_count(context);
    lfsm_transitions_t* transition = lfsm_get_transition_table(context);
    lfsm_transitions_t** transition_lookup = lfsm_get_transition_lookup_table(context);
    uint8_t current_state, last_state, current_event, last_event;

    if ((transition == NULL) || (transition_lookup == 0)) return LFSM_ERROR;

    // fill first element
    *transition_lookup = transition;
    last_state = transition->current_state;
    last_event = transition->event;
    transition_lookup++;
    transition++;

    // fill other elements
    for (int i = 1 ; i < transition_count ; i++) {
        current_state = transition->current_state;
        current_event = transition->event;
        if ((last_state != current_state) || (last_event != current_event)) {
            *transition_lookup = transition;
            transition_lookup++;
            last_state = transition->current_state;
            last_event = transition->event;
        }
        transition++;
    }
    return LFSM_OK;
}

lfsm_return_t lfsm_fill_state_function_lookup_table(lfsm_t context) {
    lfsm_state_functions_t* state_table;
    lfsm_state_functions_t** state_lookup_table;
    uint8_t state_offset;

    state_table = lfsm_get_state_function_table(context);
    state_offset = lfsm_get_state_min(context);
    state_lookup_table = lfsm_get_state_function_lookup_table(context);

    *(state_lookup_table + state_table->state - state_offset) = state_table;

    return LFSM_OK;
}


int lfsm_always() {
    return 1;
}

void* lfsm_user_data(lfsm_t context) {
    return context->user_data;
}

#ifdef TEST
lfsm_transitions_t* lfsm_get_transition_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->transition_table;
}
int lfsm_get_transition_count(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->transition_count;
}
lfsm_transitions_t** lfsm_get_transition_lookup_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->transition_lookup_table;
}
lfsm_state_functions_t* lfsm_get_state_function_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->functions_table;
}
int lfsm_get_state_function_count(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->state_func_count;
}
lfsm_state_functions_t** lfsm_get_state_function_lookup_table(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->function_lookup_table;
}
int lfsm_get_state_min(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->state_number_min;
}
int lfsm_get_state_max(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->state_number_max;
}
int lfsm_get_event_min(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->event_number_min;
}
int lfsm_get_event_max(lfsm_t context) {
    lfsm_context_t* details = context;
    return details->event_number_max;
}
#endif
