#include <stdio.h>
#include "lovely_fsm.h"

void print_transition_table(lfsm_t context) {
    lfsm_transitions_t* transition = lfsm_get_transition_table(context);
    lfsm_state_functions_t* state_functions = lfsm_get_state_function_table(context);
    int transition_count = lfsm_get_transition_count(context);

    printf("\nTransition Table for LFSM @%d\n", (int)context);
    printf("%2d transitions, transition table @%u, state table @%u\n", transition_count, (int)transition, (int)state_functions);
    printf("| STATE | EVENT | CONDITION_FUNC | STATE |\n");
    printf("|----------------------------------------|\n");
    for (int i = 0 ; i < transition_count; i++, transition++) {
        uint8_t old_state = transition->current_state;
        uint8_t new_state = transition->next_state;
        uint8_t event = transition->event;
        int condition_addr = (int)transition->condition;
        printf("| %5d | %5d | %14d | %5d |\n", old_state, event, condition_addr, new_state);
    }
    printf("\n");
}

void print_transition_lookup_table(lfsm_t context) {
    lfsm_transitions_t* transition = lfsm_get_transition_table(context);
    int min_state = lfsm_get_state_min(context);
    int max_state = lfsm_get_state_max(context);
    int min_event = lfsm_get_event_min(context);
    int max_event = lfsm_get_event_max(context);
    int lookup_size = (max_state - min_state + 1) * (max_event - min_event + 1);
    int lookup_address = lfsm_get_transition_lookup_table(context);

    printf("\nLookup Table for LFSM @%d\n", (int)context);
    printf("%d possible combinations, lookup table @%u\n", lookup_size, lookup_address);
    printf("transition table @%u\n", (int)transition);
    printf("| EVENT "); for (int i = min_event; i <= max_event; i++) printf("| %12u ", i);
    printf("|\n|-STATE-|------------------------------------------------|\n");
    for (int state = min_state ; state <= max_state; state++) {
        printf("| %5u |", state);
        for (int event = min_event ; event <= max_event ; event++) {
            printf(" %12u |", &(*(lfsm_transitions_t*)lookup_address));
            lookup_address += sizeof(lfsm_transitions_t*);
        }
        printf("\n");
    }
}
