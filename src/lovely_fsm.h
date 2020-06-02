#ifndef __LOVELY_FSM_H
#define __LOVELY_FSM_H

#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))
#define lfsm_init(transition_table, state_table, user_data) lfsm_init_func(&transition_table[0], ARRAYSIZE(transition_table), &state_table[0], ARRAYSIZE(state_table), &user_data)
// --------------------------------------------
#include <stdint.h>
#include "lovely_fsm_config.h"
/* -----------------------------------------------------------------------------
 *  User exposed pointer to LFSM context and function return types
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t* lfsm_t;
typedef enum lfsm_return_t {
    LFSM_OK,
    LFSM_MORE_QUEUED,
    LFSM_ERROR,
} lfsm_return_t;

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
typedef buffer_handle_type (*lfsm_buf_init_func_t)(buf_data_info_t*);
#else
typedef buffer_handle_type (*lfsm_buf_init_func_t)(DATA_TYPE*);
#endif
typedef uint8_t (*lfsm_buf_is_full_func_t)(buffer_handle_type);
typedef uint8_t (*lfsm_buf_is_empty_func_t)(buffer_handle_type);
typedef DATA_TYPE* (*lfsm_buf_read_func_t)(buffer_handle_type);
typedef uint8_t (*lfsm_buf_add_func_t)(buffer_handle_type);

typedef struct lfsm_buf_callbacks_t {
    lfsm_buf_init_func_t     init;
    lfsm_buf_add_func_t      add;
    lfsm_buf_read_func_t     read;
    lfsm_buf_is_empty_func_t is_empty;
    lfsm_buf_is_full_func_t  is_full;
 } lfsm_buf_callbacks_t;

lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);
lfsm_t lfsm_init_func(lfsm_transitions_t* transitions, \
                        int trans_count,\
                        lfsm_state_functions_t* states,\
                        int state_count,\
                        void* user_data);
void* lfsm_user_data(lfsm_t context);


#ifdef TEST
lfsm_transitions_t* lfsm_get_transition_table(lfsm_t context);
int lfsm_get_transition_count(lfsm_t context);
lfsm_state_functions_t* lfsm_get_state_function_table(lfsm_t context);
int lfsm_get_state_function_count(lfsm_t context);
#endif


#endif // __LOVELY_FSM_H
