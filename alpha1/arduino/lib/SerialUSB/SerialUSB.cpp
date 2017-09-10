/*

Class based on async 2-directional serial comms example given 
http://forum.arduino.cc/index.php?topic=225329.msg1810764#msg1810764
Written by member Robin2, posted Mar 12th 2014

*/

#include "Arduino.h"
#include "SerialUSB.h"

SerialUSB::SerialUSB() {
	//pinMode(13, OUTPUT); // the onboard LED
	//pinMode(8, OUTPUT);  // offboard LED for GUI interaction checking
	//pinMode(5, OUTPUT);  // offboard LED
	//pinMode(7, OUTPUT);
	
	//The below functions were included in the example code but have been moved to the main program
	/*
	Serial.begin(9600);
	
	debugToPC("Arduino Ready from ArduinoPC.ino");
	
	blinkLED(5); // just so we know it's alive THIS FUNCTION BREAKS THE CLASS!!!
	*/
}

//===========================================

void SerialUSB::getSerialData() {

     // Receives data into tempBuffer[]
     //   saves the number of bytes that the PC said it sent - which will be in tempBuffer[1]
     //   uses decodeHighBytes() to copy data from tempBuffer to dataRecvd[]
     
     // the Arduino program will use the data it finds in dataRecvd[]

  if(Serial.available() > 0) {

    byte x = Serial.read();
    if (x == startMarker) { 
      bytesRecvd = 0; 
      inProgress = true;
      // blinkLED(2);
      // debugToPC("start received");
    }
      
    if(inProgress) {
      tempBuffer[bytesRecvd] = x;
      bytesRecvd ++;
    }

    if (x == endMarker) {
      inProgress = false;
      allReceived = true;
      
        // save the number of bytes that were sent
      dataSentNum = tempBuffer[1];
  
      decodeHighBytes();
    }
  }
}

//========================================================

void SerialUSB::sendSerialData( char name[], char value[] ) {
	// sends off the dataset received from outside the class
	dataSendCount = 0;
	
	// trim down size of value[] to fit our maxMessage size
	strncpy(value, value, maxMessage - sizeof(name) - 1);

	// append name[] and value[] into dataSend[]
	int i = 0;
	for (i = 0; i < nameLength; i++) {
		dataSend[i] = name[i];
	}
	for (i = nameLength; i < maxMessage; i++) {
		dataSend[i] = value[i - nameLength];
	}
	
	dataSendCount = i;

	// send off the message UNCOMMENT THIS FOR TESTING PYTHON RPI PROGRAM
	dataToPC();

	
	// testing serial pass
	// debugToPC(name);
	// debugToPC(value);
	
	//Serial.write("----------bottom of sendSerialData");
	//Serial.println();
}

//========================================================

void SerialUSB::processData() {

    // processes the data that is in dataRecvd[]

  if (allReceived) {
  
      // for demonstration just copy dataRecvd to dataSend
    dataSendCount = dataRecvCount;
    for (byte n = 0; n < dataRecvCount; n++) {
       dataSend[n] = dataRecvd[n];
    }

    dataToPC();

    delay(100);
    allReceived = false; 
  }
}

//=======================================

void SerialUSB::decodeHighBytes() {

  //  copies to dataRecvd[] only the data bytes i.e. excluding the marker bytes and the count byte
  //  and converts any bytes of 253 etc into the intended numbers
  //  Note that bytesRecvd is the total of all the bytes including the markers
  dataRecvCount = 0;
  for (byte n = 2; n < bytesRecvd - 1 ; n++) { // 2 skips the start marker and the count byte, -1 omits the end marker
    byte x = tempBuffer[n];
    if (x == specialByte) {
       // debugToPC("FoundSpecialByte");
       n++;
       x = x + tempBuffer[n];
    }
    dataRecvd[dataRecvCount] = x;
    dataRecvCount ++;
  }
}

//=========================================

void SerialUSB::dataToPC() {

      // expects to find data in dataSend[]
      //   uses encodeHighBytes() to copy data to tempBuffer
      //   sends data to PC from tempBuffer
    encodeHighBytes();

    Serial.write(startMarker);
    Serial.write(dataSendCount);
    Serial.write(tempBuffer, dataTotalSend);
    Serial.write(endMarker);
	
	Serial.write("----------bottom of dataToPC");
	Serial.println();
}

//============================================

void SerialUSB::encodeHighBytes() {
  // Copies to temBuffer[] all of the data in dataSend[]
  //  and converts any bytes of 253 or more into a pair of bytes, 253 0, 253 1 or 253 2 as appropriate
  dataTotalSend = 0;
  for (byte n = 0; n < dataSendCount; n++) {
    if (dataSend[n] >= specialByte) {
      tempBuffer[dataTotalSend] = specialByte;
      dataTotalSend++;
      tempBuffer[dataTotalSend] = dataSend[n] - specialByte;
    }
    else {
      tempBuffer[dataTotalSend] = dataSend[n];
    }
    dataTotalSend++;
  }
  Serial.write("----------bottom of encodeHighBytes");
  Serial.println();
}

//=============================================

void SerialUSB::debugToPC( char arr[]) {
    byte nb = 0;
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(arr);
    Serial.write(endMarker);
    Serial.println();
}

//==============================================

void SerialUSB::debugToPC( byte num) {
    byte nb = 0;
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(num);
    Serial.write(endMarker);
}

//==============================================

void SerialUSB::blinkLED(byte numBlinks) {
    for (byte n = 0; n < numBlinks; n ++) {
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
	  
    }
	digitalWrite(8, HIGH);
}

