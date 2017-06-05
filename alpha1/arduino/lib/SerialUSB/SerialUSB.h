/*

Class based on serial comms example given 
http://forum.arduino.cc/index.php?topic=225329.msg1810764#msg1810764
Written by member Robin2, posted Mar 12th 2014

*/

#ifndef SerialUSB_h
#define SerialUSB_h

#include "Arduino.h"
#include <string.h>

class SerialUSB
{
	public:
		SerialUSB();
		void getSerialData();
		void sendSerialData( char name[], char value[] );
		void processData();
		void debugToPC( char arr[]);
	private:	
		void decodeHighBytes();
		void dataToPC();
		void encodeHighBytes();
		void debugToPC( byte num);
		void blinkLED( byte numBlinks);
		
		#define startMarker 254
		#define endMarker 255
		#define specialByte 253
		#define maxMessage 16
		#define nameLength 3		// Character length of the channel names being read. Commonly 'DO', 'EC', 'ORP', 'pH'

		byte bytesRecvd = 0;
		byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
		byte dataRecvCount = 0;


		byte dataRecvd[maxMessage]; 
		byte dataSend[maxMessage];  
		byte tempBuffer[maxMessage];

		byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
		byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

		boolean inProgress = false;
		boolean startFound = false;
		boolean allReceived = false;
		
		// variables added to allow for assembling messages to send



};
		
#endif
