
#define DCSBIOS_IRQ_SERIAL
//#define DCSBIOS_DEFAULT_SERIAL
#include "DcsBios.h"


// THIS IS CODE FOR THE ARMAMENT PANEL IN a4 skyhawk in DCS.
// Right now it only has the pylon switches. but the logic will be used for more as
// i add it.
const int pin2 = 2; // the pin on the arduino we wired
const int pin3 = 3;
const int pin4 = 4;
const int pin5 = 5;
const int pin6 = 6;

const int nr_armstation_pins = 5;
int switch_pins[nr_armstation_pins] = {pin2, pin3, pin4, pin5, pin6};

// this is to remember the states of the real switches.
// we start by setting all switchstates to off.
int physical_armstation_switch_states[nr_armstation_pins] = {0,0,0,0,0};

// this is to keep the states of the DCS switches
// we start by setting these switch states to off.
int dcs_armstation_switch_states[nr_armstation_pins] = {0,0,0,0,0};

// this is to keep track of when we polled dcs last.
unsigned long lastTime = 0;

// is DCS running a A-4E-C plane? we use this bool to check 
// during the game. we start by setting it to false
bool a4_loaded = false;

// this code gets invoked by DCS BIOS when plane changes
void on_plane_change(char* new_value){
  // we check if the plane we are running in DCS is the skyhawk.
  if (strcmp(new_value, "A-4E-C") == 0) {
    a4_loaded = true;
  } else {
    a4_loaded = false;
  }
}
// this is the code that will invokle the function above for plane change
// this is a part of the DCS library that we use to read the state from the
// game to the arduino. In this case, this lets us get the name of the plane.
// its event driven, and will run whenever the value in DCS changes.
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, on_plane_change);

// function to read the physical switch state
// remember this code is for a switch that is "on" when it is 
// electrically closed to ground.
int read_pysical_switch_state(int switch_pin){
  return digitalRead(switch_pin) == LOW ? 1 : 0;
}

// function to send the physical state of a switch to DCS
// so DCS can update the virtual switch in the cockpit.
// it takes the  state of the switch and what amrmament pylon it 
// is connected to (armstation)
void send_armstation_state(int state, int armstation){
  lastTime = millis();
  switch (armstation) {
    case 1:
      sendDcsBiosMessage("ARM_STATION1", state ? "1" : "0");
      break;
    case 2:
      sendDcsBiosMessage("ARM_STATION2", state ? "1" : "0");
      break;
    case 3:
      sendDcsBiosMessage("ARM_STATION3", state ? "1" : "0");
      break;
    case 4:
      sendDcsBiosMessage("ARM_STATION4", state ? "1" : "0");
      break;
    case 5:
      sendDcsBiosMessage("ARM_STATION5", state ? "1" : "0");
      break;
  }
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
// setting a function to set the dcs switch state (one of the functions above.
DcsBios::IntegerBuffer armStation1StateBuffer(0x8500, 0x0020, 5, arm_station1_state_change);
DcsBios::IntegerBuffer armStation2StateBuffer(0x8500, 0x0040, 6, arm_station2_state_change);
DcsBios::IntegerBuffer armStation3StateBuffer(0x8500, 0x0080, 7, arm_station3_state_change);
DcsBios::IntegerBuffer armStation4StateBuffer(0x8500, 0x0100, 8, arm_station4_state_change);
DcsBios::IntegerBuffer armStation5StateBuffer(0x8500, 0x0200, 9, arm_station5_state_change);

// now i automatically will have the dcs_armstation_switchX_state
// and i can check if they are the same as the physical_switchX_state. 

void setup() {
  // set the pinMode to pullup for the pins we use
  // remember we have wired it so we connect it to ground. (most people do that)
  for (int i = 0; i < nr_armstation_pins; i++){
    pinMode(switch_pins[i], INPUT_PULLUP);
  }

  // start the dcs stuff 
  DcsBios::setup();

  // read the physical switch state
  for (int i = 0; i < nr_armstation_pins; i++){
    physical_armstation_switch_states[i] = read_pysical_switch_state(switch_pins[i]);
  }
}

void loop() {
  DcsBios::loop();

  // we read the physical_armstation_switchX_state into current_X
  for (int i = 0; i < nr_armstation_pins; i++){
    int current_state = read_pysical_switch_state(switch_pins[i]);
    // now we see if that has changed, and if so send it to DCS
    if (current_state != physical_armstation_switch_states[i]){
      // it has changed, so we set the physical armstation to the new value
      physical_armstation_switch_states[i] = current_state;
      // and we send the value to DCS so it also updates the virtual switch.
      send_armstation_state(physical_armstation_switch_states[i], i + 1);
    }
  }

  // so now we have to find when DCS has changed the switch, but we have not flipped the physical switch.
  // and then we reset it to what it actually is physically, but only if we are int he A4 plane
  // else, if we are not in dcs, we will continue to try and set it, since we can not poll the value 
  // and verify.. thus we end up in a loop. We only try and read the value from DCS, if we known we are
  // in the skyhawk in DCS.
  // we also make sure to wait 250 milliseconds, so we dont trigger some unknown race thing.
  if (a4_loaded){
    for (int i = 0; i < nr_armstation_pins; i++){
      if (dcs_armstation_switch_states[i] != physical_armstation_switch_states[i] && millis() - lastTime > 250){
        send_armstation_state(physical_armstation_switch_states[i], i + 1);
      }
    }
  }
}

// thats it. It works really welll.
// now to update with some more switches than just the pylon switches. but that will be for a later evning.

