#ifndef __LOVELY_FSM_H
#define __LOVELY_FSM_H

#define lfsm_init(transition_table, state_table) lfsm_init_func()
// --------------------------------------------
#include <stdint.h>
#include "lovely_fsm_config.h"
/* -----------------------------------------------------------------------------
 *  Context pointer to identify a state machine.
 * -------------------------------------------------------------------------- */
typedef struct lfsm_context_t* lfsm_t;

/* -----------------------------------------------------------------------------
 *  To be defined by the user
 * -------------------------------------------------------------------------- */
typedef struct lfsm_state_functions_t {
    int state;
    void (*on_entry) ( lfsm_t );
    void (*on_run  ) ( lfsm_t );
    void (*on_exit ) ( lfsm_t );
} lfsm_state_functions_t;

typedef struct lfsm_transitions_t {
    int current_state;
    int event;
    int (*condition)( lfsm_t );
    int next_state;
} lfsm_transitions_t;

/* -----------------------------------------------------------------------------
 *  
 * -------------------------------------------------------------------------- */
typedef enum lfsm_return_t {
    LFSM_OK,
    LFSM_MORE_QUEUED,
    LFSM_ERROR,
} lfsm_return_t;

/* -----------------------------------------------------------------------------
 *  
 * -------------------------------------------------------------------------- */
typedef buffer_handle_type* (*lfsm_buf_init_func_t)(DATA_TYPE*);
typedef uint8_t (*lfsm_buf_is_full_func_t)(buffer_handle_type*);
typedef uint8_t (*lfsm_buf_is_empty_func_t)(buffer_handle_type*);
typedef DATA_TYPE* (*lfsm_buf_read_func_t)(buffer_handle_type*);
typedef uint8_t (*lfsm_buf_add_func_t)(buffer_handle_type*);

typedef struct lfsm_buf_callbacks_t {
    lfsm_buf_init_func_t     init;
    lfsm_buf_add_func_t      add;
    lfsm_buf_read_func_t     read;
    lfsm_buf_is_empty_func_t is_empty;
    lfsm_buf_is_full_func_t  is_full;
 } lfsm_buf_callbacks_t;

lfsm_return_t fsm_add_event(lfsm_t context, uint8_t event);



#endif // __LOVELY_FSM_H
