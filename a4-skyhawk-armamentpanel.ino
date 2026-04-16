#define DCSBIOS_IRQ_SERIAL
//#define DCSBIOS_DEFAULT_SERIAL
#include "DcsBios.h"


// THIS IS CODE FOR THE ARMAMENT PANEL IN a4 skyhawk in DCS.
// Right now it only has the pylon switches. but the logic will be used for more as
// i add it.
const char* stations[] = {"ARM_STATION1","ARM_STATION2","ARM_STATION3","ARM_STATION4","ARM_STATION5"};
// the pins they are connected to.
const int pin2 = 2;
const int pin3 = 3;
const int pin4 = 4;
const int pin5 = 5;
const int pin6 = 6;

const int nr_armstation_pins = 5;
int switch_pins[nr_armstation_pins] = {pin2, pin3, pin4, pin5, pin6};

// this is to remember the states of the real switches.
int physical_armstation_switch_states[nr_armstation_pins] = {0,0,0,0,0};

// this is to keep the states of the DCS switches
int dcs_armstation_switch_states[nr_armstation_pins] = {0,0,0,0,0};

// this is to keep track of when we polled dcs last.
unsigned long lastTime = 0;

// is DCS running a A-4E-C plane? we use this bool to check 
// during the game
bool a4_loaded = false;
// this code gets invoked by DCS BIOS when plane changes
void on_plane_change(char* new_value){
  if (strcmp(new_value, "A-4E-C") == 0) {
    a4_loaded = true;
  } else {
    a4_loaded = false;
  }
}
// this is the code that will invokle the function above for plane change
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, on_plane_change);


// function to read the physical switch state
// remember this code is for a switch that is "on" when it is 
// electrically closed to ground.
int read_pysical_switch_state(int switch_pin){
  return digitalRead(switch_pin) == LOW ? 1 : 0;
}

// send the physical state to DCS
void send_armstation_state(int state, int armstation){
  lastTime = millis();
  // just so we keep ourselves to the five existing armstations on the a4
  if (armstation < 0 || armstation > 5) return;  
  sendDcsBiosMessage(stations[armstation], state ? "1" : "0");
}

// this is the funtion to set the state of a DCS switch state
// called below from the DcsBios::IntegerBuffer
void arm_station1_state_change(unsigned int newValue){
  dcs_armstation_switch_states[0] = newValue ? 1 : 0;
}
void arm_station2_state_change(unsigned int newValue){
  dcs_armstation_switch_states[1] = newValue ? 1 : 0;
}
void arm_station3_state_change(unsigned int newValue){
  dcs_armstation_switch_states[2] = newValue ? 1 : 0;
}
void arm_station4_state_change(unsigned int newValue){
  dcs_armstation_switch_states[3] = newValue ? 1 : 0;
}
void arm_station5_state_change(unsigned int newValue){
  dcs_armstation_switch_states[4] = newValue ? 1 : 0;
}

// this will trigger when there is a change of this in dcs
// setting a function to set the dcs switch state
DcsBios::IntegerBuffer armStation1StateBuffer(0x8500, 0x0020, 5, arm_station1_state_change);
DcsBios::IntegerBuffer armStation2StateBuffer(0x8500, 0x0040, 6, arm_station2_state_change);
DcsBios::IntegerBuffer armStation3StateBuffer(0x8500, 0x0080, 7, arm_station3_state_change);
DcsBios::IntegerBuffer armStation4StateBuffer(0x8500, 0x0100, 8, arm_station4_state_change);
DcsBios::IntegerBuffer armStation5StateBuffer(0x8500, 0x0200, 9, arm_station5_state_change);

// now i automatically will have the dcs_armstation_switchX_state
// and i can check if they are the same as the physical_switchX_state. 

void setup() {
  // set the pinMode to pullup for the pins we use
  for (int i = 0; i < nr_armstation_pins; i++){
    pinMode(switch_pins[i], INPUT_PULLUP);
  }

  // start the dcs stuff 
  DcsBios::setup();

  // read the physical switch
  for (int i = 0; i < nr_armstation_pins; i++){
    physical_armstation_switch_states[i] = read_pysical_switch_state(switch_pins[i]);
  }
}

void loop() {
  DcsBios::loop();

  // we only care if we are connected to the a4 plane
  if (a4_loaded){
    // we read the physical_armstation_switchX_state into current_X
    for (int i = 0; i < nr_armstation_pins; i++){
      int current_state = read_pysical_switch_state(switch_pins[i]);
      // now we see if that has changed, and if so send it to DCS
      if (current_state != physical_armstation_switch_states[i]){
        physical_armstation_switch_states[i] = current_state;
        send_armstation_state(physical_armstation_switch_states[i], i );
      }
    }

    // so now we have to find when DCS has changed the switch, but we have not flipped the switch.
    // and then we reset it to what it actually is physically, 
    for (int i = 0; i < nr_armstation_pins; i++){
      if (dcs_armstation_switch_states[i] != physical_armstation_switch_states[i] && millis() - lastTime > 250){
        send_armstation_state(physical_armstation_switch_states[i], i );
      }
    }
  }
}

