// WhiteBox Labs -- Tentacle Shield -- I2C asynchronous example
// https://www.whiteboxes.ch/tentacle
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set
// - Change the variables TOTAL_CIRCUITS, channel_ids and channel_names to reflect your setup
// - Set host serial terminal to 9600 baud
//
//---------------------------------------------------------------------------------------------
//
//
//---------------------------------------------------------------------------------------------

#include <Wire.h>                              // enable I2C.
#include <SerialUSB.h>						   // my homebrew serial communication library

#define TOTAL_CIRCUITS 4                       // <-- CHANGE THIS | set how many I2C circuits are attached to the Tentacle
#define MAX_DATA 30							   // The char array size of our sensor reading data

const unsigned int baud_host  = 9600;        // set baud rate for host serial monitor(pc/mac/other)
const unsigned int send_readings_every = 5000; // set at what intervals the readings are sent to the computer (NOTE: this is not the frequency of taking the readings!)
unsigned long next_serial_time;

char sensordata[MAX_DATA];                    // A character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;               // We need to know how many characters bytes have been received
byte code = 0;                                // used to hold the I2C response code.
byte in_char = 0;                             // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.

int channel_ids[] = {97, 98, 99, 100};        // <-- CHANGE THIS. A list of I2C ids that you set your circuits to.
char *channel_names[] = {"DO", "ORP", "PH", "EC"};   // <-- CHANGE THIS. A list of channel names (must be the same order as in channel_ids[]) - only used to designate the readings in serial communications
char readings[TOTAL_CIRCUITS][MAX_DATA];     // A jagged array of chars to hold the readings of each channel
int channel = 0;                              // INT pointer to hold the current position in the channel_ids/channel_names array

const unsigned int reading_delay = 1000;      // time to wait for the circuit to process a read command. datasheets say 1 second.
unsigned long next_reading_time;              // holds the time when the next reading should be ready from the circuit
boolean request_pending = false;              // whether or not we're waiting for a reading

const unsigned int blink_frequency = 250;     // the frequency of the led blinking, in milliseconds
unsigned long next_blink_time;                // holds the next time the led should change state
boolean led_state = LOW;                      // keeps track of the current led state

SerialUSB transmit;							  // create object 'transmit' from class SerialUSB to communicate with RPi

void setup() {
  pinMode(13, OUTPUT);                        // set the led output pin
  Serial.begin(baud_host);	              	  // Set the hardware serial port.
  delay(500);							  	  // slight pause to allow the RPi to respond
  transmit.debugToPC("Arduino Ready"); 		  // send serial begin signal to RPi
  Wire.begin();			              		  // enable I2C port.
  // transmit.debugToPC("I2C executed");
  next_serial_time = millis() + send_readings_every;  // calculate the next point in time we should do serial communications
}



void loop() {
  do_sensor_readings();
  do_serial();
  // Do other stuff here (Blink Leds, update a display, etc)


  blink_led();
}



// blinks a led on pin 13 asynchronously
void blink_led() {
  if (millis() >= next_blink_time) {                  // is it time for the blink already?
    led_state = !led_state;                           // toggle led state on/off
    digitalWrite(13, led_state);                      // write the led state to the led pin
    next_blink_time = millis() + blink_frequency;     // calculate the next time a blink is due
  }
}



// do serial communication in a "asynchronous" way
void do_serial() {
  if (millis() >= next_serial_time) {                // is it time for the next serial communication?
    transmit.getSerialData();						 // use 'transmit' object to pull target value serial data from RPi
    // transmit.debugToPC("serial get done");

	for (int i = 0; i < TOTAL_CIRCUITS; i++) {       // loop through all the sensors
      transmit.sendSerialData( channel_names[i], readings[i] );       // use 'transmit' object to push sensor readings to RPi
      // transmit.debugToPC("----------serial send done");
	}
    next_serial_time = millis() + send_readings_every;
  }
}


// take sensor readings in a "asynchronous" way
void do_sensor_readings() {
  if (request_pending) {                          // is a request pending?
    if (millis() >= next_reading_time) {          // is it time for the reading to be taken?
      receive_reading();                          // do the actual I2C communication
      // transmit.debugToPC("----- sensor read done");
    }
  } else {                                        // no request is pending,
    channel = (channel + 1) % TOTAL_CIRCUITS;     // switch to the next channel (increase current channel by 1, and roll over if we're at the last channel using the % modulo operator)
    request_reading();                            // do the actual I2C communication
  }
}



// Request a reading from the current channel
void request_reading() {
  request_pending = true;
  Wire.beginTransmission(channel_ids[channel]); // call the circuit by its ID number.
  Wire.write('r');        		        // request a reading by sending 'r'
  Wire.endTransmission();          	        // end the I2C data transmission.
  next_reading_time = millis() + reading_delay; // calculate the next time to request a reading
}



// Receive data from the I2C bus
void receive_reading() {
  sensor_bytes_received = 0;                        // reset data counter
  memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

  Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
  code = Wire.read();

  while (Wire.available()) {          // are there bytes to receive?
    in_char = Wire.read();            // receive a byte.

    if (in_char == 0) {               // if we see that we have been sent a null command.
      Wire.endTransmission();         // end the I2C data transmission.
      break;                          // exit the while loop, we're done here
    }
    else {
      sensordata[sensor_bytes_received] = in_char;  // load this byte into our array.
      sensor_bytes_received++;
    }
  }

  switch (code) {                  	    // switch case based on what the response code is.
    case 1:                       	    // decimal 1  means the command was successful.
      strncpy(readings[channel], sensordata, MAX_DATA - 1);
      break;                        	    // exits the switch case.

    case 2:                        	    // decimal 2 means the command has failed.
      strncpy(readings[channel], "error: command failed", MAX_DATA - 1);
      break;                         	    // exits the switch case.

    case 254:                      	    // decimal 254  means the command has not yet been finished calculating.
      strncpy(readings[channel], "reading not ready", MAX_DATA -1);
      break;                         	    // exits the switch case.

    case 255:                      	    // decimal 255 means there is no further data to send.
      strncpy(readings[channel], "error: no data", MAX_DATA - 1);
      break;                         	    // exits the switch case.
  }
  request_pending = false;                  // set pending to false, so we can continue to the next sensor
}
