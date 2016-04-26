#include <user_config.h>
#include <SmingCore/SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	//#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
	//#define WIFI_PWD "PleaseEnterPass"
#endif

// ... and/or MQTT username and password
#ifndef MQTT_USERNAME
	#define MQTT_USERNAME ""
	#define MQTT_PWD ""
#endif

// ... and/or MQTT host and port
#ifndef MQTT_HOST
	#define MQTT_HOST "level1"
	#define MQTT_PORT 1883
#endif

/* (My) User settings */
#define MQTT_PUBLISH "I/Mobile/Mobile/Mobile/PIR"	// Percorso di pubblicazione dei dati MQTT
#define MQTT_SUBSCRIBE "O/Mobile/Mobile/Mobile/#"			// Percorso di lettura dei dati MQTT
#define MQTT_ID "PIR"										// Identificatore

#define INTPIN 2   // GPIO 2

// Forward declarations
void startMqttClient();
void publishMessage();	// Dicharazione
void checkPin();		// " "

Timer procTimer;
int Message=0;
int MemoMessage=Message;

// MQTT client
// For quick check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
//MqttClient mqtt(MQTT_HOST, MQTT_PORT, onMessageReceived);
MqttClient mqtt(MQTT_HOST, MQTT_PORT);

// Publish our message
void publishMessage()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect
	
	Serial.println("Let's publish message now!");
	Serial.println(String(MQTT_PUBLISH) + " ID:" + String(MQTT_ID) + " Valore:" + String(Message));
	mqtt.publish(MQTT_PUBLISH, "{ \"ID\" : \"" + String(MQTT_ID) + "\", \"Valore\" : \"" + String(Message) + "\" }"); // or publishWithQoS
	Serial.println("******************************************");

}

// Run MQTT client
void startMqttClient()
{
	//procTimer.stop();									// Questa non c'era
	if(!mqtt.setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	mqtt.connect("esp8266", MQTT_USERNAME, MQTT_PWD);

}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Run MQTT client
	Serial.println("Start MQTT client ...");
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(3 * 100, checkPin).start(); // ogni 300 millisecondi

}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

void IRAM_ATTR interruptHandler()
{
	// This is not very good idea - make actions in interrupt handler (it's just an example)
	// Better set some flag and check it in main code (timers and etc) or read actual pins state and save it to variable
	// Interrupt processing code should be as short as possible.
	// flagInterruptOccurred = true;
	
	Message=digitalRead(INTPIN);
}

void checkPin()
{
	if(MemoMessage != Message)
	{
		publishMessage();
		MemoMessage=Message;
	}
}


void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Interrupt
	attachInterrupt(INTPIN, interruptHandler, CHANGE);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start

}
