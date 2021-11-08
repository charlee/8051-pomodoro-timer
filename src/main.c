#include <8052.h>
#include <stdlib.h>



#define SEGS P1
#define GROUP P3

#define BTN_DEC P2_0
#define BTN_INC P2_1
#define BTN_RUN P2_2
#define BEEP P2_3

// #define start_timer() { is_running = 1; TR1 = 1; }
// #define stop_timer() { is_running = 0; TR1 = 0; }

#define MODE_50MIN 0
#define MODE_10MIN 1

#define POWER_DOWN_COUNTER_VALUE 50000L * 170



// 2ms
#define T0_INTERVAL 2000
#define reset_T0() { TL0 = (65536 - T0_INTERVAL) % 256; TH0 = (65536 - T0_INTERVAL) / 256; }

// 100ms
#define T1_INTERVAL 31470
#define T1_MULTIPLIER 2
#define reset_T1() { TL1 = (65536 - T1_INTERVAL) % 256; TH1 = (65536 - T1_INTERVAL) / 256; }

__code unsigned char sevenseg_hex[] = {
  0x3F, 0x06, 0x5B,  0x4F,
  0x66, 0x6D, 0x7D, 0x07,
  0x7F, 0x6F, 0x77, 0x7C,
  0x39, 0x5E, 0x79, 0x71
};



void init();
void shutdown();
void delay(int n){
  while (n--);
}

void beep() {
  unsigned int n;
  unsigned char i;
  unsigned char b = 0;

  for (i = 0; i < 10; i++) {

    BEEP = b;

    b = (b == 1) ? 0 : 1;

    n = 20000;
    while (n--) {
      if (BTN_RUN == 0 || BTN_INC == 0 || BTN_DEC == 0) {
        BEEP = 1;
        n = 50000;
        while (n--);
        return;
      }
    }
  }

  BEEP = 1;
}


unsigned long power_down_counter = POWER_DOWN_COUNTER_VALUE;
unsigned int timer_100msecs;
unsigned char is_running = 0;
unsigned char is_time_up = 0;

unsigned char digits[4];

void show() {
  unsigned int secs = timer_100msecs / 10;
  unsigned char min = secs / 60;
  unsigned char sec = secs % 60;

  // since GROUP connection is G4=P3.0, G3 = P3.2, G2=P3.4, G3=P3.6, we need to reverse the digits here
  digits[3] = min / 10;
  digits[2] = min % 10;
  digits[1] = sec / 10;
  digits[0] = sec % 10;
}

// mode == 0: 50min, mode == 1: 10min
unsigned char mode;
unsigned char timer_changed;

void start_timer() {
  is_time_up = 0;
  is_running = 1;

  // reset power down counter
  power_down_counter = POWER_DOWN_COUNTER_VALUE;

  TR1 = 1;
}

void stop_timer() {
  is_time_up = 0;
  is_running = 0;
  TR1 = 0;
}

void reset_timer() {
  ET0 = 0;      // must disable T0 interrupt when changing timer_100msecs, otherwise may render wrong time
  timer_100msecs = (mode == MODE_10MIN) ? 6000 : 30000;
  show();
  ET0 = 1;
}

void switch_mode() {
  if (mode == MODE_50MIN) {
    mode = MODE_10MIN;
  } else {
    mode = MODE_50MIN;
  }
}

void inc_timer() {
  ET0 = 0;      // must disable T0 interrupt when changing timer_100msecs, otherwise may render wrong time
  if (mode == MODE_10MIN) {
    timer_100msecs = (timer_100msecs > 53400) ? 54000 : timer_100msecs + 600;
  } else {
    timer_100msecs = (timer_100msecs > 51000) ? 54000 : timer_100msecs + 3000;
  }
  show();
  ET0 = 1;
}

void dec_timer() {
  ET0 = 0;      // must disable T0 interrupt when changing timer_100msecs, otherwise may render wrong time
  if (mode == MODE_10MIN) {
    timer_100msecs = (timer_100msecs < 1200) ? 600 : timer_100msecs - 600;
  } else {
    timer_100msecs = (timer_100msecs < 3600) ? 600 : timer_100msecs - 3000;
  }
  show();
  ET0 = 1;
}




void main() {

  int counter = 0;

  // a counter for testing for long press
  unsigned char long_press_counter;

  init();

  mode = MODE_50MIN;
  timer_changed = 0;
  reset_timer();

  while(1) {

    if (BTN_RUN == 0) {
      // debounce
      delay(1000);

      long_press_counter = 100;

      // test for press-and-hold
      while (BTN_RUN == 0) {
        delay(1000);
        long_press_counter--;
        if (long_press_counter == 0) {

          // switch between 10min profile and 50min profile
          if (!timer_changed) {
            switch_mode();
          }
          reset_timer();
          timer_changed = 0;

          // wait until button is released, then break the current loop 
          while (BTN_RUN == 0);

          // debounce
          delay(1000);
          break;
        }
      }

      // short press, start/stop the timer
      if (long_press_counter > 0) {
        // start/stop the counter
        if (is_running) {
          stop_timer();
        } else {
          start_timer();
        }

        timer_changed = 1;
      }
    }

    // if btn_inc is pressed when the timer is not running, change the set time
    if (BTN_INC == 0 && is_running == 0) {
      // debounce
      delay(1000);

      while (BTN_INC == 0);

      inc_timer();

      timer_changed = 1;
    }

    // if btn_inc is pressed when the timer is not running, change the set time
    if (BTN_DEC == 0 && is_running == 0) {
      // debounce
      delay(1000);

      while (BTN_DEC == 0);

      dec_timer();

      timer_changed = 1;
    }

    if (is_time_up && is_running) {
      // stop the timer
      stop_timer();
      beep();

      // reset timer
      switch_mode();
      reset_timer();
      show();

    }

    // power-down if not running for 5 min
    if (!is_running && (timer_100msecs == 30000 || timer_100msecs == 6000)) {
      power_down_counter--;
      if (power_down_counter == 0) {

        // power down
        shutdown();
      }
    }
  }
}

void shutdown() {
  TR0 = 0;
  TR1 = 0;
  EA = 0;

  GROUP = 0xff;
  SEGS = 0xff;

  PCON = PD;
}

void init() {
  // Turn off all digits
  GROUP = 0xff;
  SEGS = 0xff;

  // Set buttons as inputs
  BTN_RUN = 1;
  BTN_INC = 1;
  BTN_DEC = 1;

  // Stop beep
  BEEP = 1;

  // setup timer 0 and timer 1
  TMOD = 0x11;
  EA = 1;
  ET0 = 1;
  ET1 = 1;

  // Start timer 0
  TR0 = 1;
}


unsigned char scan_pos = 0;


/**
 * timer0 is used to handle the 7seg scanning.
 */
void timer0() __interrupt 1 {

  ET1 = 0;

  SEGS = 0xff;

  // GROUP connections are G4=P3.0, G3=P3.2, G2=P3.4, G3=P3.6
  GROUP = ~(0x01 << (scan_pos << 1));

  if (scan_pos == 2 && (timer_100msecs % 10 >= 5)) {
    // show blinking dot
    SEGS = ~(sevenseg_hex[digits[scan_pos]] | 0x80);
  } else {
    SEGS = ~sevenseg_hex[digits[scan_pos]];
  }

  scan_pos++;
  if (scan_pos == 4) {
    scan_pos = 0;
  }

  reset_T0();

  ET1 = 1;
}

unsigned char t1_multiplier = T1_MULTIPLIER;

/**
 * timer1 is used for countdown timer
 */
void timer1() __interrupt 3 {
  ET0 = 0;

  if (t1_multiplier > 0) {
    t1_multiplier--;
  } else {
    if (timer_100msecs > 0) {
      timer_100msecs--;
    } else {
      // set time_up flag, tell main loop to stop the timer
      is_time_up = 1;
    }

    t1_multiplier = T1_MULTIPLIER;
  }

  show();
  reset_T1();
  ET0 = 1;
}