/*
 Name:		JKJson_sources.ino
 Created:	2/20/2026 1:05:40 PM
 Author:	Jari
*/
#include "JKJson.h"

// the setup function runs once when you press reset or power the board
void setup() 
{
	Serial.begin(19200);
	delay(1000);

	// Simple example to create JSON 

	char buf[100]; // Set required buffer size. Start with big one and check with json.size(); for real rquired size
	JKJson json(buf, sizeof(buf));

	json.clear(); // Not necessary required, but good method
	json.beginObject();
	json.addItem(F("HouseINTemp"), 23);
	json.finalize();

	Serial.println(json.size());
	//Serial.write(buf, json.size()); // Raw data print. Can use Serial.println also...
	JKJson::printPrettyJson(buf, Serial, 2); // Pretty printing...


}

// the loop function runs over and over again until power down or reset
void loop() 
{


  
}
