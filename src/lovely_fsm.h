#ifndef __LOVELY_FSM_H
#define __LOVELY_FSM_H

// move these to lfsm_config.h
#define LFSM_MAX_COUNT      5
#define LFSM_EV_QUEUE_SIZE  5

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
typedef enum lfsm_states_t;
typedef enum lfsm_events_t;

typedef struct lfsm_state_functions_t {
    lfsm_states_t state;
    void (*on_entry) ( lfsm_t );
    void (*on_run  ) ( lfsm_t );
    void (*on_exit ) ( lfsm_t );
} lfsm_state_functions_t;

typedef struct lfsm_transitions_t {
    lfsm_states_t current_state;
    lfsm_events_t event;
    lfsm_states_t next_state;
} lfsm_transitions_t;


/* -----------------------------------------------------------------------------
 *  
 * -------------------------------------------------------------------------- */
typedef enum lfsm_return_t {
    LFSM_OK,
    LFSM_MORE_QUEUED,
    LFSM_ERROR,
} lfsm_return_t;

#endif // __LOVELY_FSM_H
