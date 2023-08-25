#include "RemoteSerial.h"

void RemoteSerialClass::begin(AsyncWebServer *server, const char* url){
	_server = server;
	_ws = new AsyncWebSocket("/remoteserialws"); // this hardcoded as it is referenced by the webpage

	_server->on(url, HTTP_GET, [](AsyncWebServerRequest *request){
	   request->send_P(200, F("text/html"), WEBPAGE_HTML);   
	});

	_ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
		switch (type){
			case WS_EVT_CONNECT:
				break;
			case WS_EVT_DISCONNECT:
				break;
			case WS_EVT_DATA:
				if (_incomingMessageHandler != NULL){
					_incomingMessageHandler(data, len);
				}
				break;
		}
	});

	_server->addHandler(_ws);
}

void RemoteSerialClass::setIncomingMessageHandler(IncomingMessageHandler handler){
	_incomingMessageHandler = handler;
}

size_t RemoteSerialClass::write(uint8_t m) {
	_ws->textAll((const char*) &m, 1);
	return 1;
}

size_t RemoteSerialClass::write(const uint8_t* buffer, size_t size) {
	_ws->textAll((const char*) buffer, size);
	return size;
}

void RemoteSerialClass::cleanupClients(){
	_ws->cleanupClients();
}

RemoteSerialClass RemoteSerial;