// https://github.com/ccutrer/balboa_worldwide_app/blob/master/doc/protocol.md
// Reference: https://github.com/ccutrer/balboa_worldwide_app/wiki

// Please install the needed dependencies:
// CircularBuffer
// PubSubClient

// TODO:
// Implement Existing Client Request/Response
// Handle non-CTS Client IDs

// +12V RED
// GND  BLACK
// A    YELLOW
// B    WHITE

#define WIFI_SSID "YOURSSID"
#define WIFI_PASSWORD ""
#define BROKER ""
#define BROKER_LOGIN ""
#define BROKER_PASS ""

#define STRON String("ON").c_str()
#define STROFF String("OFF").c_str()

//#define TX485 D1
#define RLY1  D7
#define RLY2  D8

#include <ESP8266WiFi.h>

#include <CircularBuffer.h>
CircularBuffer<uint8_t, 35> Q_in;
CircularBuffer<uint8_t, 35> Q_out;

#include <ESP8266WebServer.h>   // Local WebServer used to serve the configuration portal
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

#include <PubSubClient.h>       // MQTT client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

extern uint8_t crc8();
extern void ID_request();
extern void ID_ack();
extern void rs485_send();


uint8_t x, i, j;

uint8_t last_state_crc = 0x00;
uint8_t send = 0x00;
uint8_t settemp = 0x00;
uint8_t id = 0x00;

unsigned long lastrx = 0;

struct {
  uint8_t jet1 : 1;
  uint8_t jet2 : 1;
  uint8_t blower : 1;
  uint8_t light : 1;
  uint8_t restmode: 1;
  uint8_t highrange: 1;
  uint8_t padding : 2;
} SpaState;

void _yield() {
  yield();
  mqtt.loop();
}

void print_msg(CircularBuffer<uint8_t, 35> &data) {
  String s;
  //for (i = 0; i < (Q_in[1] + 2); i++) {
  for (i = 0; i < data.size(); i++) {
    x = data[i];
    if (x < 0x0A) s += "0";
    s += String(x, HEX);
    s += " ";
  }
  mqtt.publish("Spa/msg", s.c_str());
  _yield();
}

///////////////////////////////////////////////////////////////////////////////

void hardreset() {
  ESP.wdtDisable();
  while (1) {};
}

void reconnect() {
  int oldstate = mqtt.state();
  boolean connection = false;

  // Loop until we're reconnected
  if (!mqtt.connected()) {
    // Attempt to connect
    if (BROKER_PASS == "") {
      connection = mqtt.connect(String(String("Spa") + String(millis())).c_str());
    }
    else {
      connection = mqtt.connect("Spa1", BROKER_LOGIN, BROKER_PASS);
    }


    if (connection) { //, "Spa/node", 0, true, "OFF")) {
      mqtt.publish("Spa/node", "ON", true);
      mqtt.publish("Spa/debug", "RECONNECT");
      mqtt.publish("Spa/debug", String(millis()).c_str());
      mqtt.publish("Spa/debug", String(oldstate).c_str());

      // ... and resubscribe
      mqtt.subscribe("Spa/command");
      mqtt.subscribe("Spa/temperature");
      mqtt.subscribe("Spa/heatmode");
      mqtt.subscribe("Spa/highrange");
      mqtt.subscribe("Spa/jets/#");
      mqtt.subscribe("Spa/blower");
      mqtt.subscribe("Spa/light");
      mqtt.subscribe("Spa/relay/#");

      last_state_crc = 0x00;
    }
  }
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte * p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  String topic = String(p_topic);

  mqtt.publish("Spa/debug", topic.c_str());
  _yield();

  // handle message topic
  if (topic.startsWith("Spa/relay/")) {
    bool newstate = 0;

    if (payload.equals("ON")) newstate = LOW;
    else if (payload.equals("OFF")) newstate = HIGH;

    if (topic.charAt(10) == '1') {
      pinMode(RLY1, INPUT);
      delay(25);
      pinMode(RLY1, OUTPUT);
      digitalWrite(RLY1, newstate);
    }
    else if (topic.charAt(10) == '2') {
      pinMode(RLY2, INPUT);
      delay(25);
      pinMode(RLY2, OUTPUT);
      digitalWrite(RLY2, newstate);
    }
  } else if (topic.equals("Spa/command")) {
    if (payload.equals("reset")) hardreset();
  } else if (topic.equals("Spa/heatmode")) {
    if (payload.equals("ON") && SpaState.restmode == 1) send = 0x51;
    else if (payload.equals("OFF") && SpaState.restmode == 0) send = 0x51;
  } else if (topic.equals("Spa/light")) {
    if (payload.equals("ON") && SpaState.light == 0) send = 0x11;
    else if (payload.equals("OFF") && SpaState.light == 1) send = 0x11;
  } else if (topic.startsWith("Spa/jets/")) {
    if (topic.charAt(9) == '1')  {
      if (payload.equals("ON") && SpaState.jet1 == 0) send = 0x04;
      else if (payload.equals("OFF") && SpaState.jet1 == 1) send = 0x04;
    }
    else if (topic.charAt(9) == '2') {
      if (payload.equals("ON") && SpaState.jet2 == 0) send = 0x05;
      else if (payload.equals("OFF") && SpaState.jet2 == 1) send = 0x05;
    }
  } else if (topic.equals("Spa/blower")) {
    if (payload.equals("ON") && SpaState.blower == 0) send = 0x0C;
    else if (payload.equals("OFF") && SpaState.blower == 1) send = 0x0C;
  } else if (topic.equals("Spa/highrange")) {
    if (payload.equals("ON") && SpaState.highrange == 0) send = 0x50;
    else if (payload.equals("OFF") && SpaState.highrange == 1) send = 0x50;
  } else if (topic.equals("Spa/temperature")) {
    // Get new set temperature
    double d = payload.toDouble();
    if (d > 0) d *= 2; // Convert to internal representation
    settemp = d;
    send = 0xff;
  }
}

///////////////////////////////////////////////////////////////////////////////

void setup() {
  // Begin RS485 in listening mode -> no longer required with new RS485 chip
  //pinMode(TX485, OUTPUT);
  //digitalWrite(TX485, LOW);

  pinMode(RLY1, OUTPUT);
  digitalWrite(RLY1, HIGH);
  pinMode(RLY2, OUTPUT);
  digitalWrite(RLY2, HIGH);

  // Spa communication, 115.200 baud 8N1
  Serial.begin(115200);

  // give Spa time to wake up after POST
  for (uint8_t i = 0; i < 5; i++) {
    delay(1000);
    yield();
  }

  Q_in.clear();
  Q_out.clear();

  {
    WiFi.setOutputPower(20.5); // this sets wifi to highest power
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long timeout = millis() + 10000;

    while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
      yield();
    }

    // Reset because of no connection
    if (WiFi.status() != WL_CONNECTED) {
      hardreset();
    }
  }

  httpUpdater.setup(&httpServer, "admin", "");
  httpServer.begin();
  MDNS.begin("Spa");
  MDNS.addService("http", "tcp", 80);

  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(callback);
  mqtt.setKeepAlive(10);
  mqtt.setSocketTimeout(20);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) ESP.restart();
  if (!mqtt.connected()) reconnect();
  httpServer.handleClient();
  _yield();

  // Read from Spa RS485
  if (Serial.available()) {
    x = Serial.read();
    Q_in.push(x);

    // Drop until SOF is seen
    if (Q_in.first() != 0x7E) Q_in.clear();
    lastrx = millis();
  }

  // DEBUG: mqtt.publish("Spa/rcv", String(x).c_str()); _yield();

  // Double SOF-marker, drop last one
  if (Q_in[1] == 0x7E && Q_in.size() > 1) Q_in.pop();

  // Complete package
  //if (x == 0x7E && Q_in[0] == 0x7E && Q_in[1] != 0x7E) {
  if (x == 0x7E && Q_in.size() > 2) {
    //print_msg();

    // Unregistered or yet in progress
    if (id == 0) {
      if (Q_in[2] == 0xFE) print_msg(Q_in);

      // FE BF 02: got new client ID
      if (Q_in[2] == 0xFE && Q_in[4] == 0x02) {
        id = Q_in[5];
        if (id > 0x2F) id = 0x2F;

        ID_ack();
        mqtt.publish("Spa/id", String(id).c_str());
      }

      // FE BF 00: Any new clients?
      if (Q_in[2] == 0xFE && Q_in[4] == 0x00) {
        ID_request();
      }
    } else if (Q_in[2] == id) {
      if (Q_in[4] == 0x06) {
        // id BF 06: Ready to Send
        if (send == 0xff) {
          // 0xff marks dirty temperature for now
          Q_out.push(id);
          Q_out.push(0xBF);
          Q_out.push(0x20);
          Q_out.push(settemp);
        } else if (send == 0x00) {
          // A Nothing to Send message is sent by a client immediately after a Clear to Send message if the client has no messages to send.
          Q_out.push(id);
          Q_out.push(0xBF);
          Q_out.push(0x07);
        } else {
          // Send toggle commands
          Q_out.push(id);
          Q_out.push(0xBF);
          Q_out.push(0x11);
          Q_out.push(send);
          Q_out.push(0x00);
        }

        rs485_send();
        send = 0x00;
      }
    }

    // Normal package handling
    // FF AF 13: Status Update - Packet index offset 5
    if (Q_in[2] == 0xFF && Q_in[4] == 0x13) {
      if (last_state_crc != Q_in[Q_in[1]]) {
        String s;
        double d = 0.0;
        double c = 0.0;

        // DEBUG for finding meaning:
        //print_msg(Q_in);

        // 25: Set Temperature
        d = Q_in[25] / 2;
        if (Q_in[25] % 2 == 1) d += 0.5;
        mqtt.publish("Spa/state/target", String(d, 2).c_str());

        // 7: Actual temperature
        if (Q_in[7] != 0xFF) {
          d = Q_in[7] / 2;
          if (Q_in[7] % 2 == 1) d += 0.5;
          if ((d > c * 1.2) || (d < c * 0.8)) d = c; //remove spurious readings greater or less than 20% away from previous read
          mqtt.publish("Spa/state/temperature", String(d, 2).c_str());
          c = d;
        } else {
          d = 0;
        }
        // REMARK Move upper publish to HERE to get 0 for unknown temperature

        // 8: Hour & 9: Minute => Time
        if (Q_in[8] < 10) s = "0"; else s = "";
        s = String(Q_in[8]) + ":";
        if (Q_in[9] < 10) s += "0";
        s += String(Q_in[9]);
        mqtt.publish("Spa/state/time", s.c_str());

        // 10: Heating Mode
        switch (Q_in[10]) {
          case 0: mqtt.publish("Spa/state/heatingmode", STRON);
          case 3:
            SpaState.restmode = 0;
            break;
          case 1: mqtt.publish("Spa/state/heatingmode", STROFF);
            SpaState.restmode = 1;
            break;
        }

        // 15: Flags Byte 10 / Heat status, Temp Range
        d = bitRead(Q_in[15], 4);
        if (d == 0) mqtt.publish("Spa/state/heatstate", STROFF);
        else if (d == 1 || d == 2) mqtt.publish("Spa/state/heatstate", STRON);

        d = bitRead(Q_in[15], 2);
        if (d == 0) {
          mqtt.publish("Spa/state/highrange", STROFF);
          SpaState.highrange = 0;
        } else if (d == 1) {
          mqtt.publish("Spa/state/highrange", STRON);
          SpaState.highrange = 1;
        }

        // 16: Flags Byte 11
        if (bitRead(Q_in[16], 1) == 1) {
          mqtt.publish("Spa/state/jets1", STRON);
          SpaState.jet1 = 1;
        } else {
          mqtt.publish("Spa/state/jets1", STROFF);
          SpaState.jet1 = 0;
        }

        if (bitRead(Q_in[16], 3) == 1) {
          mqtt.publish("Spa/state/jets2", STRON);
          SpaState.jet2 = 1;
        } else {
          mqtt.publish("Spa/state/jets2", STROFF);
          SpaState.jet2 = 0;
        }

        // 18: Flags Byte 13
        if (bitRead(Q_in[18], 1) == 1)
          mqtt.publish("Spa/state/circ", STRON);
        else
          mqtt.publish("Spa/state/circ", STROFF);

        if (bitRead(Q_in[18], 2) == 1) {
          mqtt.publish("Spa/state/blower", STRON);
          SpaState.blower = 1;
        } else {
          mqtt.publish("Spa/state/blower", STROFF);
          SpaState.blower = 0;
        }
        // 19: Flags Byte 14
        if (Q_in[19] == 0x03) {
          mqtt.publish("Spa/state/light", STRON);
          SpaState.light = 1;
        } else {
          mqtt.publish("Spa/state/light", STROFF);
          SpaState.light = 0;
        }

        last_state_crc = Q_in[Q_in[1]];

        // Publish own relay states
        s = "OFF";
        if (digitalRead(RLY1) == LOW) s = "ON";
        mqtt.publish("Spa/state/relay1", s.c_str());

        s = "OFF";
        if (digitalRead(RLY2) == LOW) s = "ON";
        mqtt.publish("Spa/state/relay2", s.c_str());
      }
    } else {
      // DEBUG for finding meaning
      //if (Q_in[2] & 0xFE || Q_in[2] == id)
      //print_msg(Q_in);
    }

    // Clean up queue
    _yield();
    Q_in.clear();
  }

  // Long time no receive
  if (millis() - lastrx > 5000) {
    hardreset();
  }
}
