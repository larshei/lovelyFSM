#include <stdio.h>

void print_transition_table(lfsm_t context) {
    lfsm_transitions_t* transition = lfsm_get_transition_table(context);
    int transition_count = lfsm_get_transition_count(context);

    printf("\n--- Transition Table for LFSM @%d with %2d transitions ---\n", (int)transition, transition_count);
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
