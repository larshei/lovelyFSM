#ifndef __LOVELY_FSM_H
#define __LOVELY_FSM_H

#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))
#define lfsm_init(transition_table, state_table, buf_callbacks, user_data, initial_state) lfsm_init_func(&transition_table[0], ARRAYSIZE(transition_table), &state_table[0], ARRAYSIZE(state_table), buf_callbacks, user_data, initial_state)
// --------------------------------------------
#include <stdint.h>
#include "lovely_fsm_config.h"
/* -----------------------------------------------------------------------------
 *  User exposed pointer to LFSM context and function return types
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t* lfsm_t;
typedef enum lfsm_return_t {
    LFSM_OK,
    LFSM_NOP,
    LFSM_MORE_QUEUED,
    LFSM_ERROR,
} lfsm_return_t;

#define LFSM_INVALID  0xFE

/* -----------------------------------------------------------------------------
 *  For convenience
 * -------------------------------------------------------------------------- */
int lfsm_always();
#define always lfsm_always

/* -----------------------------------------------------------------------------
 *  To be defined by the user
 * -------------------------------------------------------------------------- */
typedef struct lfsm_state_functions_t {
    int state;
    lfsm_return_t (*on_entry) ( lfsm_t );
    lfsm_return_t (*on_run  ) ( lfsm_t );
    lfsm_return_t (*on_exit ) ( lfsm_t );
} lfsm_state_functions_t;

typedef struct lfsm_transitions_t {
    int current_state;
    int event;
    int (*condition)( lfsm_t );
    int next_state;
} lfsm_transitions_t;


/* -----------------------------------------------------------------------------
 *  Buffer Setup
 * -------------------------------------------------------------------------- */
#ifdef USE_LOVELY_BUFFER
typedef uint8_t (*lfsm_buf_sys_init_func_t)();
typedef buffer_handle_type (*lfsm_buf_init_func_t)(buf_data_info_t*);
#else
typedef void (*lfsm_buf_sys_init_func_t)();
typedef buffer_handle_type (*lfsm_buf_init_func_t)(DATA_TYPE*);
#endif

typedef uint8_t (*lfsm_buf_is_full_func_t)(buffer_handle_type);
typedef uint8_t (*lfsm_buf_is_empty_func_t)(buffer_handle_type);
typedef DATA_TYPE (*lfsm_buf_read_func_t)(buffer_handle_type);
typedef uint8_t (*lfsm_buf_add_func_t)(buffer_handle_type, DATA_TYPE);

typedef struct lfsm_buf_callbacks_t {
    lfsm_buf_sys_init_func_t  system_init;
    lfsm_buf_init_func_t      init ;
    lfsm_buf_add_func_t       add ;
    lfsm_buf_read_func_t      read ;
    lfsm_buf_is_empty_func_t  is_empty ;
    lfsm_buf_is_full_func_t   is_full ;
} lfsm_buf_callbacks_t;

lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);
lfsm_return_t lfsm_deinit(lfsm_t context);
lfsm_return_t lfsm_run(lfsm_t context);

lfsm_t lfsm_init_func(lfsm_transitions_t* transitions, \
                        int trans_count,\
                        lfsm_state_functions_t* states,\
                        int state_count,\
                        lfsm_buf_callbacks_t buffer_callbacks, \
                        void* user_data, \
                        uint8_t initial_state);

void* lfsm_user_data(lfsm_t context);
uint8_t lfsm_get_state(lfsm_t context);


#ifdef USE_LOVELY_BUFFER
lfsm_return_t lfsm_set_lovely_buf_callbacks(lfsm_buf_callbacks_t* callbacks);
#endif

#ifdef TEST
lfsm_transitions_t* lfsm_get_transition_table(lfsm_t context);
int lfsm_get_transition_count(lfsm_t context);
lfsm_state_functions_t* lfsm_get_state_function(struct lfsm_context_t* fsm, uint8_t state);
lfsm_state_functions_t* lfsm_get_state_function_table(lfsm_t context);
int lfsm_get_state_function_count(lfsm_t context);
lfsm_transitions_t** lfsm_get_transition_lookup_table(lfsm_t context);
lfsm_state_functions_t** lfsm_get_state_function_lookup_table(lfsm_t context);
int lfsm_get_state_min(lfsm_t context);
int lfsm_get_state_max(lfsm_t context);
int lfsm_get_event_min(lfsm_t context);
int lfsm_get_event_max(lfsm_t context);
uint8_t lfsm_set_state(lfsm_t context, uint8_t state);
uint8_t lfsm_get_state_func_count(lfsm_t context);
lfsm_transitions_t* lfsm_get_transition_from_lookup(lfsm_t context, uint8_t event);
uint8_t lfsm_read_event_queue_element(lfsm_t context, uint8_t index);
uint8_t lfsm_no_event_queued(struct lfsm_context_t* fsm);
uint8_t lfsm_read_event(lfsm_t context);
#endif


#endif // __LOVELY_FSM_H
