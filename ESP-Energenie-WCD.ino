/*********
Gemaakt voor in de Energenie op de NodeMCU V3
Basic idea from https://randomnerdtutorials.com/esp8266-web-server-with-arduino-ide/
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Subscribe to the following topic for switching the power sockets/relais
const char* MQTT_TOPIC_ENERGENIE = "huis/ESP-Energenie/+/out";
const char* strClientID = "ESP-Energenie-WCD";

// Replace with your network credentials
const char* SSID		= "WifiSSID";
const char* PASSWORD    = "WifiPassword";
const char* MQTT_SERVER = "192.168.5.248";

// Set web server port number to 80
WiFiServer httpServer(80);

// Variable to store the HTTP request
String header;

#define DEBUG		1	// set to 1 to display each loop() run and PIR trigger
#define RELAY_OFF	 0
#define RELAY_ON		1

// Auxiliar variables to store the current output state
String relais1State = "off";
String relais2State = "off";
String relais3State = "off";
String relais4State = "off";
String relais5State = "off";
String relais6State = "off";

// Assign output variables to GPIO pins
const int relais1 = 16; //GPIO 16
const int relais2 = 13; //GPIO 13
const int relais3 = 12; //GPIO 12
const int relais4 = 14; //GPIO 14
const int relais5 = 5; //GPIO 5
const int relais6 = 4; //GPIO 4

WiFiClient espClient;
WiFiClient httpClient;

PubSubClient mqttClient(espClient);


void switchRelay(char number, char status) {
	//"Relais 1 off"
#if DEBUG
	Serial.print("Relais ");
	Serial.print(number, DEC);
	if (status == RELAY_OFF) Serial.println(": off");
	else										 Serial.println(": on");
#endif

	switch (number) {
		case 1: //Relay 1
			if (status == RELAY_OFF) {
				relais1State = "off";
			} else {
				relais1State = "on";
			}
			digitalWrite(relais1, status);
			break;
		case 2: //Relay 2
			if (status == RELAY_OFF) {
				relais2State = "off";
			} else {
				relais2State = "on";
			}
			digitalWrite(relais2, status);
			break;
		case 3: //Relay 3
			if (status == RELAY_OFF) {
				relais3State = "off";
			} else {
				relais3State = "on";
			}
			digitalWrite(relais3, status);
			break;
		case 4: //Relay 4
			if (status == RELAY_OFF) {
				relais4State = "off";
			} else {
				relais4State = "on";
			}
			digitalWrite(relais4, status);
			break;
		case 5: //Relay 5: inverted signal
			if (status == RELAY_OFF) {
				relais5State = "off";
				digitalWrite(relais5, 1);
			} else {
				relais5State = "on";
				digitalWrite(relais5, 0);
			}
			break;
		case 6: //Relay 6: inverted signal
			if (status == RELAY_OFF) {
				relais6State = "off";
				digitalWrite(relais6, 1);
			} else {
				relais6State = "on";
				digitalWrite(relais6, 0);
			}
			break;
	}
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
	char number;
	char data;
	char status;

	number = topic[19];
	data = (char)payload[0];

	//Topic: "huis/ESP-Energenie/+/out"
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("]: Socket: ");
	Serial.print(number);
	Serial.print(" : ");
	Serial.println(data);

	if ((data == '1') || (data == 1))
		status = RELAY_ON;
	else
		status = RELAY_OFF;

	switchRelay(number - '0', status);
}

void reconnect() {
	for (int i = 0; i < 5; i++) {
		// Check if we're reconnected
		if (!mqttClient.connected()) {
			Serial.print("Attempting MQTT connection...");
			// Attempt to connect
			if (mqttClient.connect(strClientID)) {
				Serial.println("connected");

				//Once connected subscribe to the topic
				mqttClient.subscribe(MQTT_TOPIC_ENERGENIE);
			} else {
				Serial.print("failed, rc=");
				Serial.print(mqttClient.state());
				Serial.println(" try again in 5 seconds");
				// Wait 5 seconds before retrying
				delay(5000);
			}
		} else {
			//Connected! Break out of the loop
			break;
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial.print("[");
	Serial.print(strClientID);
	Serial.println("]");

	// Initialize the output variables as outputs
	pinMode(relais1, OUTPUT);
	pinMode(relais2, OUTPUT);
	pinMode(relais3, OUTPUT);
	pinMode(relais4, OUTPUT);
	pinMode(relais5, OUTPUT);
	pinMode(relais6, OUTPUT);

	// Set relais default ON
	switchRelay(1, RELAY_ON);
	switchRelay(2, RELAY_ON);
	switchRelay(3, RELAY_ON);
	switchRelay(4, RELAY_ON);
	switchRelay(5, RELAY_ON);
	switchRelay(6, RELAY_OFF);

	//WIFIconnect();

	mqttClient.setServer(MQTT_SERVER, 1883);
	mqttClient.setCallback(mqttCallback);

	//Start the http server
	httpServer.begin();
}

void displayHTMLpage(void) {
	if (httpClient.connected()) {

		// Display the HTML web page
		httpClient.println("<!DOCTYPE html><html>");
		httpClient.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
		httpClient.println("<title>Energenie NodeMCU</title>");
		httpClient.println("<meta http-equiv=\"refresh\" content=\"5; URL=/\">");
		httpClient.println("<link rel=\"icon\" href=\"data:,\">");
		// CSS to style the on/off buttons
		// Feel free to change the background-color and font-size attributes to fit your preferences
		httpClient.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
		httpClient.println(".button { background-color: red; border: none; color: white; padding: 16px 40px; border-radius: 15px;");
		httpClient.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
		httpClient.println(".button2 {background-color: green;}</style></head>");

		// Web Page Heading
		httpClient.println("<body><h3>Energenie ESP-12S</h3>");

		// Display current state, and ON/OFF buttons for Relais 1
		httpClient.println("<p>Socket 1: RPi-Infra (HomeAssistant) - State: " + relais1State + "</p>");
		if (relais1State=="off") {
			httpClient.println("<p><a href=\"/1/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/1/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 2
		httpClient.println("<p>Socket 2: RPi-HomeLogic - State: " + relais2State + "</p>");
		if (relais2State=="off") {
			httpClient.println("<p><a href=\"/2/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/2/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 3
		httpClient.println("<p>Socket 3: CB2-Live - State: " + relais3State + "</p>");
		if (relais3State=="off") {
			httpClient.println("<p><a href=\"/3/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/3/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 4
		httpClient.println("<p>Socket 4 - Wooning-NAS - State: " + relais4State + "</p>");
		if (relais4State=="off") {
			httpClient.println("<p><a href=\"/4/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/4/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 5
		httpClient.println("<p>Socket 5 - POE Switch - State: " + relais5State + "</p>");
		if (relais5State=="off") {
			httpClient.println("<p><a href=\"/5/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/5/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 6
		httpClient.println("<p>Socket 6 - Licht Kerst Buiten (Voordeur) - State: " + relais6State + "</p>");
		if (relais6State=="off") {
			httpClient.println("<p><a href=\"/6/on\"><button class=\"button\">OFF</button></a></p>");
		} else {
			httpClient.println("<p><a href=\"/6/off\"><button class=\"button button2\">ON</button></a></p>");
		}

		// Display current state, and ON/OFF buttons for Relais 6
		httpClient.println("<p>Power reset - Shutdown Socket 1 till 3 (power on after 20 sec)</p>");
		httpClient.println("<p><a href=\"/power_reset\"><button class=\"button button2\">RESET</button></a></p>");

		httpClient.println("</body></html>");

		// The HTTP response ends with another blank line
		httpClient.println();

	}
}

void mqttLoop() {
	//Reconnect if the MQTT client is disconnected
	if (!mqttClient.connected()) {
		WIFIconnect();
		reconnect();
	}

	//Give the processor to the MQTT service
	mqttClient.loop();
}

void WIFIconnect()
{
	delay(10);
	WiFi.disconnect();
#if DEBUG
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);
#endif
	//WiFi.mode(WIFI_AP_STA);

	WiFi.begin(SSID, PASSWORD);

		// Wait for connection
	for (int i = 0; i < 75; i++) {
		if ( WiFi.status() != WL_CONNECTED ) {
			delay(500);
#if DEBUG
			Serial.print(".");
#endif
		}
	}

#if DEBUG
	if (WiFi.status() == WL_CONNECTED) {
		Serial.println("");
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
	}
#endif

}

void loop() {
	//
	// if (WiFi.status() != WL_CONNECTED) {
	//	 WIFIconnect();
	// }

	httpClient = httpServer.available();	 // Listen for incoming clients

	if (httpClient) {														 // If a new httpClient connects,
		Serial.println("New httpClient connected.");					// print a message out in the serial port
		String currentLine = "";								// make a String to hold incoming data from the httpClient
		while (httpClient.connected()) {						// loop while the httpClient's connected
			if (httpClient.available()) {						 // if there's bytes to read from the httpClient,
				char c = httpClient.read();						 // read a byte, then
				//Serial.write(c);										// print it out the serial monitor
				header += c;
				if (c == '\n') {										// if the byte is a newline character
					// if the current line is blank, you got two newline characters in a row.
					// that's the end of the httpClient HTTP request, so send a response:
					if (currentLine.length() == 0) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the httpClient knows what's coming, then a blank line:
						httpClient.println("HTTP/1.1 200 OK");
						httpClient.println("Content-type:text/html");
						httpClient.println("Connection: close");
						httpClient.println();

						// turns the GPIOs on and off
						if (header.indexOf("GET /1/on") >= 0) {
							switchRelay(1, RELAY_ON);
						} else if (header.indexOf("GET /1/off") >= 0) {
							switchRelay(1, RELAY_OFF);
						} else if (header.indexOf("GET /2/on") >= 0) {
							switchRelay(2, RELAY_ON);
						} else if (header.indexOf("GET /2/off") >= 0) {
							switchRelay(2, RELAY_OFF);
						} else if (header.indexOf("GET /3/on") >= 0) {
							switchRelay(3, RELAY_ON);
						} else if (header.indexOf("GET /3/off") >= 0) {
							switchRelay(3, RELAY_OFF);
						} else if (header.indexOf("GET /4/on") >= 0) {
							switchRelay(4, RELAY_ON);
						} else if (header.indexOf("GET /4/off") >= 0) {
							switchRelay(4, RELAY_OFF);
						} else if (header.indexOf("GET /5/on") >= 0) {
							switchRelay(5, RELAY_ON);
						} else if (header.indexOf("GET /5/off") >= 0) {
							switchRelay(5, RELAY_OFF);
						} else if (header.indexOf("GET /6/on") >= 0) {
							switchRelay(6, RELAY_ON);
						} else if (header.indexOf("GET /6/off") >= 0) {
							switchRelay(6, RELAY_OFF);
						} else if (header.indexOf("GET /power_reset") >= 0) {
							switchRelay(1, RELAY_OFF);
							switchRelay(2, RELAY_OFF);
							switchRelay(3, RELAY_OFF);
							Serial.println("20 sec delay");
							delay(20000); //20sec
							switchRelay(1, RELAY_ON);
							switchRelay(2, RELAY_ON);
							switchRelay(3, RELAY_ON);
						}
						displayHTMLpage();

						// Break out of the while loop
						break;
					} else { // if you got a newline, then clear currentLine
						currentLine = "";
					}
				} else if (c != '\r') {	// if you got anything else but a carriage return character,
					currentLine += c;			// add it to the end of the currentLine
				}
			}

			//Serve the MQTT Client also when the httpClient is connected
			mqttLoop();
		}
		// Clear the header variable
		header = "";
		// Close the connection
		httpClient.stop();
		Serial.println("httpClient disconnected.");
		Serial.println("");
	}

	delay(100); //100ms

	//Serving the MQTT service
	mqttLoop();

}
