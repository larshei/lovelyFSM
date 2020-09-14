# lovelyFSM

lovelyFSM (lfsm) is a finite state machine library written in C, 
with the goal to make state machines easy to write, read and maintain.
You can start multiple instances of different or the same state machine.

A simple example:

``` C
// this example assumes you are using lovelyBuffer (shipped with lovelyFSM)

lfsm_transitions_t transition_table[] = {
  // STATE      EVENT             CONDITION              TRANSITION TO
  { ST_NORMAL , EV_MEASURE      , temperature_warning  , ST_WARN   },
  { ST_NORMAL , EV_MEASURE      , temperature_critical , ST_ALARM  },
  { ST_WARN   , EV_MEASURE      , temperature_okay     , ST_NORMAL },
  { ST_WARN   , EV_MEASURE      , temperature_critical , ST_ALARM  },
  { ST_ALARM  , EV_BUTTON_PRESS , temperature_okay     , ST_NORMAL },
};

lfsm_state_functions_t state_func_table[] = {
  // STATE     ON_ENTRY        ON_RUN  ON_EXIT
  {ST_NORMAL , green_led_on  , NULL  , all_leds_off },
  {ST_WARN   , yellow_led_on , NULL  , all_leds_off },
  {ST_ALARM  , red_led_on    , NULL  , all_leds_off},
};

lfsm_t lfsm_handle;
lfsm_buf_callbacks_t buffer_callbacks;
my_data_t my_data; // whatever data you want to access in state machine functions

int main(void) {
  buf_init_system();
  lfsm_set_lovely_buf_callbacks(&buffer_callbacks);
  
  lfsm_handle = lfsm_init(transition_table, state_table, buffer_callbacks, (void*)my_data, ST_IDLE);
  
  // in main program loop
  {...}
  fsm_add_event(lfsm_handle, EV_MEASURE);
  lfsm_run(lfsm_handle);
  {...}
}
```

As you can see, this is a state machine that turns an LED green, yellow or red
based on temperature thresholds. Reseting from the high-temperature alarm state
requires a button press once the temperatue has lowered.

## Library Versions

There are two versions of this library, 
[lovelyFSM](https://github.com/larshei/lovelyFSM/) and 
[lovelyFSM-light](https://github.com/huf-gda/lovelyFSM-light/).

Their usage is the same, but the way they handle data internally differs.

> if you are not sure which version to use, use the light variant.

### lovelyFSM
Targeted at execution speed. At initialization, lookup tables for state/event
combinations are created dynamically to speed up the execution process when an
event is triggered.

If you do not have memory allocation concerns and small functions to be
executed on an event with the overall goal of lower and more constant execution
time, you may prefer this variant. 

### lovelyFSM-light
Targeted at memory footprint and no dynamic allocation.

Intead of a lookup table, a simple for loop runs over the provided transition
table to find a state/event/condition match.

# Setup

## 1. The config.h file

The src/ folder contains a `lovely_fsm_config.h` file. The following fields can
be configured:

Option|Details
-----|-----
LFSM_MAX_COUNT | Memory for lovelyFSM instances is allocated statically. Define the maximum number of intances here.
LFSM_EV_QUEUE_SIZE | Each lovelyFSM instance uses a separate FIFO to asynchronically store and process events. This defines the size of the FIFO in event-elements.
USE_LOVELY_BUFFER | LovelyFSM does not provide FIFO handling by itsself. You may use a custom FIFO implementation or lovelyBuffer. 

## 2. Event buffer

The library provides memory meant to be used as a FIFO for each state machine
instance. The FIFO is where events can be asynchronously stored to later be
processed by the state machine instance. Buffer management is NOT part of
lovelyFSM. For convenience,
[lovelyBuffer](https://github.com/larshei/lovelyBuffer) is integrated as a
submodule, but you may have your own implementation to be used instead.

To use lovelyBuffer, do the following:

1. Set `USE_LOVELY_BUFFER` in `src/lovely_fsm_config.h` to `1`
2. Run
``` BASH
git submodule update --init
```
3. In your code, use `lfsm_set_lovely_buf_callbacks` to set the required
   callback functions.

### Setting up custom Buffer management

If you use lovelyBuffer, skip this section.

The data type for events is uint8_t.

Interaction with the buffer system is done through callbacks. The following
callbacks need to be provided:

callback|function|return|
-----|-----|-----
system_init|Buffer system init, NOT called by lovelyFSM.|any
init | initializes a single buffer handler | buffer handle
add | adds an element to the FIFO | 0 on success
read | returns an element from the FIFO | event-number
is_empty | Check if the buffer is empty | int!=0 if empty
is_full | Check if the buffer is full | int!=0 if full

The functions need to be passed to lovelyFSM as `lfsm_buf_callbacks_t` on init.

## 3. Creating states and events

States and events are simply uint8_t, so you may use defines or enums as
desired.
> The non-light version will create a lookup table. The lookup table depends
> on the lowest and highest index for both events and states. 
> The size is (events_max - events_min + 1) * (states_max - states_min + 1).

## 4. Transition table

The transition table describes how states can change. Each element is of type 
``` C
typedef struct lfsm_transitions_t {
    int current_state;
    int event;
    int (*condition)( lfsm_t );
    int next_state;
} lfsm_transitions_t;
```

where `condition` is any function that returns an integer and takes and
lfsm_context as an argument. A return value of 0 is treated as "condition not
fulfilled" while any other numeric value counts as "fulfilled".

We will need to pass an array `lfsm_transitions_t[]` to the state machine later.

## 5. State table

The state table defines what functions should be run when transitioning into a
state, transitioning out of a state and staying in a state.

``` C
typedef struct lfsm_state_functions_t {
    int state;
    lfsm_return_t (*on_entry) ( lfsm_t );
    lfsm_return_t (*on_run  ) ( lfsm_t );
    lfsm_return_t (*on_exit ) ( lfsm_t );
} lfsm_state_functions_t;
```
We will need to pass an array `lfsm_state_functions_t[]` to the state machine later.

## 6. Create condition and state function prototypes

As we are referencing functions in the tables above, we need to provide at
least the function prototypes.

## 7. Create a state machine instance

Create a lovelyFSM instance using

``` C
lfsm_t* lfsm_handler;
lfsm_handler = lfsm_init( transition_table, \
                          state_func_table, \
                          buffer_callbacks, \
                          &my_data, \
                          ST_NORMAL );
```

- `transition_table`: Array created in step "Transition Table"
- `state_func_table`: Array created in step "State table"
- `buffer_callbacks`: Struct of `lfsm_buf_callbacks_t`, either populated
  manually or using `lfsm_set_lovely_buf_callbacks()` when lovelyBuffer is
  used.
- `&my_data`: Pointer to any custom data that you want to be able to access in
  condition or state functions.
- `ST_NORMAL`: Initial state for the lovelyFSM instance.

The returned `lfsm_handler` will be used to identify the lovelyFSM instance.

## 8. Add an event

Add an event using 

``` C
fsm_add_event(lfsm_handler, EVENT);
```

## 9. Run / Step

In order to execute an event, use
``` C
lfsm_return_t ret;
ret = lfsm_run(lfsm_handler);
```

`ret` can be an error, `LFSM_NOOP` (no event queued), `LFSM_OK` (event run, no
more events in queue) or `LFSM_MORE_QUEUED` (event run, more events in queue).

## 10. Deinit

To deinitialize the instance use
``` C
lfsm_return_t lfsm_deinit(lfsm_handler)
```

