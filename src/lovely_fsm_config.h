#ifndef __LOVELY_FSM_CONFIG_H
#define __LOVELY_FSM_CONFIG_H

#include "../buffer_c/buf_buffer.h"

// --- maximum number of state machines (static memory allocation!) ---
#define LFSM_MAX_COUNT          3

// --- buffer functions ---
#define USE_BUFFER_C_LIB        1

#if (USE_BUFFER_C_LIB)
#define buffer_handle_type                   buf_buffer_t
#define add_to_buffer(buffer_handle, value)  buf_add_element(buffer_handle, value)
#define read_from_buffer(buffer_handle)      buf_read_element(buffer_handle)
#define buffer_full(buffer_handle)           buf_is_full(buffer_handle)
#define buffer_empty(buffer_handle)          buf_is_empty(buffer_handle)
#else
// provide your own buffer implementation!
#define buffer_handle_type                   
#define add_to_buffer(buffer_handle, value)  
#define read_from_buffer(buffer_handle)      
#define buffer_full(buffer_handle)           
#define buffer_empty(buffer_handle)          
#endif // 

#endif // __LOVELY_FSM_CONFIG_H