//#define DEBUG_VERBOSITY

// function declation for the interrupt callbacks
void irq_buzzer_1(void);
void irq_buzzer_2(void);
void irq_buzzer_3(void);
void irq_buzzer_4(void);

// // TODO: Implement state machine
// enum state {READ_INPUT, RESET_LEDS, SET_LED, MAX_STATES};
// void read_input(int);
// void reset_leds(int);
// void set_led(int);
// void (*const state_table[MAX_STATES]) (int) = {
//   read_input,
//   reset_leds,
//   set_led
// };

// TODO: Make this serial stuff a own module/class
#define SERIAL_INPUT_BUFFER_SIZE 32
char serial_input_buffer[SERIAL_INPUT_BUFFER_SIZE];
byte serial_input_index = 0;

// TODO: Make this buzzer stuff a own module/class
#define LED_1_PIN 13
#define LED_2_PIN 14
#define LED_3_PIN 15
#define LED_4_PIN 16
#define INTERRUPT_1_PIN 7
#define INTERRUPT_2_PIN 8
#define INTERRUPT_3_PIN 9
#define INTERRUPT_4_PIN 10
const int led_pin_list[4] = {LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_4_PIN};

// TODO: Make this json stuff a own module/class
#define JSON_STRING_LENGTH 128
char buzzer_json_string[JSON_STRING_LENGTH];
const char* const buzzer_json_format_string = "{\"first_buzzer\": %i, \"buzzer_1\": %i, \"buzzer_2\": %i, \"buzzer_3\": %i, \"buzzer_4\": %i}";


// TODO: Make this buzzer handling stuff a own module/class
int           first_buzzer = 0;
volatile bool buzzer_state_changed = false;
volatile int  buzzer_state = 0;
volatile int  last_buzzer_state = 0;


/**
 * Resets the serial buffer. All buffered data is cleared and deleted
 */
void reset_serial_input_buffer(void) {
  serial_input_index = 0;
  memset(serial_input_buffer,'\0', SERIAL_INPUT_BUFFER_SIZE);
  serial_input_buffer[0] = '\0';
}

/**
 * Reset the buzzer state. All data is cleared and the led of the
 * buzzers are switched of.
 */
void reset_buzzers(void) {
  buzzer_state = 0;
  last_buzzer_state = 0;
  first_buzzer = 0;
  // reset buzzer led's
  int i;
  for (i=0; i<4; i++) {
    digitalWrite(led_pin_list[i], LOW);
  }
}

/**
 *
 */
void generate_buzzer_json_string(void) {
  snprintf(buzzer_json_string, JSON_STRING_LENGTH, buzzer_json_format_string,
           first_buzzer,
           (buzzer_state >> 0) & 1, // buzzer 1
           (buzzer_state >> 1) & 1, // buzzer 2
           (buzzer_state >> 2) & 1, // buzzer 3
           (buzzer_state >> 3) & 1 // buzzer 4
           );
}

/**
 * Read a byte of incomming serial data if available and append it to the serial 
 * buffer. If the serial buffer is full, the serial buffer is reset (all buffered 
 * data is cleared and deleted).
 */
void read_serial_input(void){
  if (Serial.available() > 0) {
    if (serial_input_index <= SERIAL_INPUT_BUFFER_SIZE-1) {
      // store read data in serial_input_buffer
      serial_input_buffer[serial_input_index] = Serial.read();
      serial_input_index++;
      serial_input_buffer[serial_input_index] = '\0';
    } else {
      Serial.println("ERROR: Serial buffer full! Buffer will the reset. Please send a new command.");
      reset_serial_input_buffer();
    }
#ifdef DEBUG_VERBOSITY
    // Just for debug purpose
    Serial.print("Buzzer buffer is: '");
    Serial.print(serial_input_buffer);
    Serial.println("'");
#endif
  }
}

/**
 * Evaluate the input string read from the serial input.
 */
void evaluate_serial_buffer(void) {
  if (serial_input_buffer[serial_input_index-1] == '\n') {

    // RESET BUZZERS
    if (strcmp(serial_input_buffer, "RESET_BUZZERS\n")  == 0) {
      reset_buzzers();
      // reset serial input buffer
      reset_serial_input_buffer();
      // send serial ACK
      Serial.println("RESET_BUZZERS OK");
      return;
    } 
    if (strcmp(serial_input_buffer, "RESET_SERIAL_BUFFER\n") == 0) {
      // reset serial input buffer
      reset_serial_input_buffer();
      // send serial ACK
      Serial.println("RESET_SERIAL_BUFFER OK");
      return;
    }
    if (strncmp(serial_input_buffer, "SET_LED ", 8) == 0) {
      // COMMAND: "SET_LED [1-4] [0,1]\n"
      // FIXME: A bit of a hack to parse for fixed positions
      unsigned short led, value;

      led   = serial_input_buffer[8]  - '0';
      value = serial_input_buffer[10] - '0';

      if (led < 1 || led > 4) {
        Serial.println("ERROR: Led must be in between 1-4");
        reset_serial_input_buffer();
        return;
      }

      if (value < 0 || value > 1) {
        Serial.println("ERROR: Value must be in between 0-1");
        reset_serial_input_buffer();
        return;
      }

      // set led
      digitalWrite(led_pin_list[led-1], value);

      // reset serial buffer
      reset_serial_input_buffer();
      // send serial ACK
      Serial.println("SET_LED OK");
      return;
    }

    // No valid command
    Serial.println("ERROR: Command unknown! Buffer will be reset.");
    reset_serial_input_buffer();
  }
}

void setup() {
  // Initialize Hardware
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);

  attachInterrupt(INTERRUPT_1_PIN, irq_buzzer_1, FALLING);
  attachInterrupt(INTERRUPT_2_PIN, irq_buzzer_2, FALLING);
  attachInterrupt(INTERRUPT_3_PIN, irq_buzzer_3, FALLING);
  attachInterrupt(INTERRUPT_4_PIN, irq_buzzer_4, FALLING);
  
  Serial.begin(115200);

  // Initialize variables
  generate_buzzer_json_string();
  reset_serial_input_buffer();
  reset_buzzers();
}

void loop() {
  // TODO: Make this a function/state
  // check if an interrupt routine changed the buzzer state
  if (buzzer_state_changed) {
    // check if data of the buzzer state changed
    if (buzzer_state != last_buzzer_state) {
#ifdef DEBUG_VERBOSITY
      Serial.print("Buzzer state: ");
      Serial.println(buzzer_state);
      Serial.print("First buzzer: ");
      Serial.println(first_buzzer);
#endif

      generate_buzzer_json_string();
      Serial.println(buzzer_json_string);
      
      if (first_buzzer != 0) {
        digitalWrite(led_pin_list[first_buzzer-1], HIGH);
      }
    }
    buzzer_state_changed = false;
    last_buzzer_state = buzzer_state;
  }

  // Read data from serial
  read_serial_input();

  // Evaluate input buffer
  evaluate_serial_buffer();
}

void irq_buzzer_1(void) {
  buzzer_state_changed = true;
  buzzer_state |= 1 << 0;
  if (first_buzzer == 0) {
    first_buzzer = 1;
  }
}

void irq_buzzer_2(void) {
  buzzer_state_changed = true;
  buzzer_state |= 1 << 1;
  if (first_buzzer == 0) {
    first_buzzer = 2;
  }
}

void irq_buzzer_3(void) {
  buzzer_state_changed = true;
  buzzer_state |= 1 << 2;
  if (first_buzzer == 0) {
    first_buzzer = 3;
  }
}

void irq_buzzer_4(void) {
  buzzer_state_changed = true;
  buzzer_state |= 1 << 3;
  if (first_buzzer == 0) {
    first_buzzer = 4;
  }
}
