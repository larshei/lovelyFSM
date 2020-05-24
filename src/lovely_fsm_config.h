#ifndef __LOVELY_FSM_CONFIG_H
#define __LOVELY_FSM_CONFIG_H

#include "../buffer_c/buf_buffer.h"

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
#define USE_BUFFER_C_LIB        1

#if (USE_BUFFER_C_LIB)
// you can use the integrated buffer library ...
#define buffer_handle_type                   buf_buffer_t
#define add_to_buffer(buffer_handle, value)  buf_add_element(buffer_handle, value)
#define read_from_buffer(buffer_handle)      buf_read_element(buffer_handle)
#define buffer_full(buffer_handle)           buf_is_full(buffer_handle)
#define buffer_empty(buffer_handle)          buf_is_empty(buffer_handle)
#define get_buffer(handle, data_array)       buf_claim_and_init_buffer(data_array)
#define BUFFER_OK   BUF_OK
#else
// ... or provide your own buffer implementation!
#define buffer_handle_type                   
#define add_to_buffer(buffer_handle, value)  
#define read_from_buffer(buffer_handle)      
#define buffer_full(buffer_handle)           
#define buffer_empty(buffer_handle)          
#define get_buffer(data_array, element_count, element_size)               
#endif // 

#endif // __LOVELY_FSM_CONFIG_H
