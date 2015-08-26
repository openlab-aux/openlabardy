The following features are needed for this project:

1. Four buzzers are supported
   * There are four and only four buzzers supported!

1. Detect a buzzer hit (buzzer state handling)
   * Send json notification via serial console
   * The led of the first detected buzzer must glow

1. Reset buzzer states
   * The host can send a serial command to reset the state of the buzzers
   * All state variables regarding the buzzers must be reset
   * All led's are shut off

1. Control the led's of all buzzers
   * The host can send a serial command to set the led (on/off) of each buzzer



The Firmware on the Arduino has the following interfaces via the serial console

1. Send a json string, when a buzzer is hit and activate the led of that buzzer
   The following json is send when a buzzer is hit:
   '{"first_buzzer": %i, "buzzer_1": %i, "buzzer_2": %i, "buzzer_3": %i, "buzzer_4": %i}'

1. Reset buzzer state
   Send 'RESET_BUZZERS\n' to reset the internal states and deactivate all led's

1. Activate/Deactivate the led of a spacific buzzer

   Send 'SET_LED %i %i\n' where the first %i indicates the Buzzer(1-4)
   and the second %i indicates the state (0-1)
   
