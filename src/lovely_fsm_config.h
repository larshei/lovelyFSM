#ifndef __LOVELY_FSM_CONFIG_H
#define __LOVELY_FSM_CONFIG_H

#include "../lovelyBuffer/buf_buffer.h"

// --- maximum number of state machines (static memory allocation!) ---
#define LFSM_MAX_COUNT          3

// --- size of event queue for each state machine. Will allocate this number
// --- of ints as part of the state machine representation. If you buffer
// --- system allocates memory itsself, you should set this value to 1.  ---
#define LFSM_EV_QUEUE_SIZE      5

// --- Create a lookup table for all state/event combinations, then run a small
// --- for loop through all conditions for this state/event combination.
// --- Alternatively, do not create a jump table and run through all entries.
#define OPTIMIZE_FOR_SPEED      1

// --- buffer functions ---
#define USE_LOVELY_BUFFER       1

#if (USE_LOVELY_BUFFER)
#define buffer_handle_type                   buf_buffer_t
#define BUFFER_OK   BUF_OK
#else
#warning "Define your buffer system here!"
#define buffer_handle_type
#define BUFFER_OK
#endif

#endif // __LOVELY_FSM_CONFIG_H
