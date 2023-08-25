#ifndef RemoteSerial_h
#define RemoteSerial_h

#if defined(ESP8266)
	#include "ESP8266WiFi.h"
	#include "ESPAsyncTCP.h"
#elif defined(ESP32)
	#include "WiFi.h"
	#include "AsyncTCP.h"
#else
	#error Platform not supported
#endif
#include "ESPAsyncWebServer.h"
#include "WebPage.h"

typedef std::function<void(const uint8_t *data, size_t len)> IncomingMessageHandler;

class RemoteSerialClass : public Print {
	public:
		void begin(AsyncWebServer *server, const char* url = "/remoteserial");
		void setIncomingMessageHandler(IncomingMessageHandler handler);
		void cleanupClients();

		size_t write(uint8_t);
		size_t write(const uint8_t* buffer, size_t size);
	private:
		AsyncWebServer* _server;
		AsyncWebSocket* _ws;
		IncomingMessageHandler _incomingMessageHandler = NULL;
};

extern RemoteSerialClass RemoteSerial;

#endif