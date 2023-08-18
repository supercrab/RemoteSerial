#if defined(ESP8266)
	#include <ESP8266WiFi.h>
	#include <ESPAsyncTCP.h>
#elif defined(ESP32)
	#include <WiFi.h>
	#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <RemoteSerial.h>

// The following colour codes are just example, more can be found here:
// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
#define ANSI_RED "\x1B[0;91m"
#define ANSI_GREEN "\x1B[0;92m"
#define ANSI_YELLOW "\x1B[0;93m"
#define ANSI_BLUE "\x1B[0;94m"
#define ANSI_MAGENTA "\x1B[0;95m"
#define ANSI_CYAN "\x1B[0;96m"
#define ANSI_WHITE "\x1B[0;97m"

AsyncWebServer server(80);

const char* ssid = "TheBrain"; // Your WiFi SSID
const char* password = "kimberley"; // Your WiFi Password

// Handle any incoming messages
void messageReceived(const uint8_t *data, size_t len){
	char str[len];

	for(uint16_t i = 0; i < len; i++){
		str[i] = data[i];
	}
	str[len] = 0;

	Serial.print("Received: ");
	Serial.println(str);
}

void setup() {
	Serial.begin(115200);
	Serial.println();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!");
		return;
	}

	Serial.println("WiFi connected");
	Serial.print("http://");
	Serial.print(WiFi.localIP().toString());
	Serial.println("/remoteserial");

	RemoteSerial.setIncomingMessageHandler(messageReceived);
	RemoteSerial.begin(&server); // You can also add a custom url, e.g. /remotedebug
	server.begin();
}

void loop() {
	Serial.println("Outputing a message");
	RemoteSerial.printf("%s IP address: %s\n", ANSI_WHITE, WiFi.localIP().toString().c_str());
	RemoteSerial.printf("%s Millis: %lu\n", ANSI_MAGENTA, millis());
	RemoteSerial.printf("%s Heap: %u\n", ANSI_CYAN, ESP.getFreeHeap());

	delay(2000);
}
