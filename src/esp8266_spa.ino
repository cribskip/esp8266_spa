// https://github.com/ccutrer/balboa_worldwide_app/blob/master/doc/protocol.md
// Reference:https://github.com/ccutrer/balboa_worldwide_app/wiki

// Please install the needed dependencies:
// CircularBuffer
// PubSubClient

// TODO:
// HomeAssistant autodiscover - mostly done
// Configuration handling
// Proper states (rather than just ON/OFF)
// OTA update from Firebase
// Settings
// Implement Existing Client Request/Response
// Handle non-CTS Client IDs

// +12V RED
// GND  BLACK
// A    YELLOW
// B    WHITE

#define VERSION ""
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define BROKER ""
#define BROKER_LOGIN ""
#define BROKER_PASS ""

#define STRON String("ON").c_str()
#define STROFF String("OFF").c_str()

//HomeAssistant autodiscover
#define HASSIO true

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
  uint8_t jet1 :1;
  uint8_t jet2 :1;
  uint8_t blower :1;
  uint8_t light :1;
  uint8_t restmode:1;
  uint8_t highrange:1;
  uint8_t padding :2;
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
  mqtt.publish("Spa/node/msg", s.c_str());
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

      // ... Hassio autodiscover
    if (HASSIO) {
        //temperature
        mqtt.publish("homeassistant/binary_sensor/Spa/state/config", "{\"name\":\"Hot tub status\",\"unique_id\":\"ESP82Spa_1\",\"state_topic\":\"Spa/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"} }");
        //temperature
        mqtt.publish("homeassistant/sensor/Spa/temperature/config", "{\"name\":\"Hot tub temperature\",\"uniq_id\":\"ESP82Spa_2\",\"dev_cla\":\"temperature\",\"stat_t\":\"Spa/temperature/state\",\"unit_of_meas\":\"°C\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"]}}");
        //target_temperature
        //mqtt.publish("homeassistant/switch/Spa/target_temp/config", "{\"name\":\"Hot tub target temperature\",\"command_topic\":\"Spa/target_temp/set\",\"state_topic\":\"Spa/target_temp/state\",\"unit_of_measurement\":\"°C\"}");
        //heat mode
        mqtt.publish("homeassistant/switch/Spa/heatingmode/config", "{\"name\":\"Hot tub heating mode\",\"unique_id\":\"ESP82Spa_3\",\"command_topic\":\"Spa/heatingmode/set\",\"state_topic\":\"Spa/heatingmode/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //high range
        mqtt.publish("homeassistant/switch/Spa/highrange/config", "{\"name\":\"Hot tub high range\",\"unique_id\":\"ESP82Spa_4\",\"command_topic\":\"Spa/highrange/set\",\"state_topic\":\"Spa/highrange/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //circulation pump
        mqtt.publish("homeassistant/binary_sensor/Spa/circ/config", "{\"name\":\"Hot tub circulation pump\",\"unique_id\":\"ESP82Spa_5\",\"device_class\":\"power\",\"state_topic\":\"Spa/circ/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //heating state
        mqtt.publish("homeassistant/binary_sensor/Spa/heatstate/config", "{\"name\":\"Hot tub heating state\",\"unique_id\":\"ESP82Spa_6\",\"state_topic\":\"Spa/heatstate/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //light 1
        mqtt.publish("homeassistant/switch/Spa/light/config", "{\"name\":\"Hot tub light\",\"unique_id\":\"ESP82Spa_7\",\"command_topic\":\"Spa/light/set\",\"state_topic\":\"Spa/light/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //jets 1
        mqtt.publish("homeassistant/switch/Spa/jet/1/config", "{\"name\":\"Hot tub jet1\",\"unique_id\":\"ESP82Spa_8\",\"command_topic\":\"Spa/jet/1/set\",\"state_topic\":\"Spa/jet/1/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //jets 2
        mqtt.publish("homeassistant/switch/Spa/jet/2/config", "{\"name\":\"Hot tub jet2\",\"unique_id\":\"ESP82Spa_9\",\"command_topic\":\"Spa/jet/2/set\",\"state_topic\":\"Spa/jet/2/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");
        //blower
        mqtt.publish("homeassistant/switch/Spa/blower/config", "{\"name\":\"Hot tub blower\",\"unique_id\":\"ESP82Spa_10\",\"command_topic\":\"Spa/blower/set\",\"state_topic\":\"Spa/blower/state\",\"platform\":\"mqtt\",\"dev\":{\"ids\":[\"ESP82Spa\"],\"name\":\"Esp Spa\",\"sw\":\"0.21\"}}");

    }


      mqtt.publish("Spa/state", "ON");
      mqtt.publish("Spa/node/debug", "RECONNECT");
      mqtt.publish("Spa/node/debug", String(millis()).c_str());
      mqtt.publish("Spa/node/debug", String(oldstate).c_str());
      mqtt.publish("Spa/node/version", VERSION);
      mqtt.publish("Spa/node/flashsize", String(ESP.getFlashChipRealSize()).c_str());
      mqtt.publish("Spa/node/chipid", String(ESP.getChipId()).c_str());

      // ... and resubscribe
      mqtt.subscribe("Spa/command");
      mqtt.subscribe("Spa/target_temp/set");
      mqtt.subscribe("Spa/heatmode/set");
      mqtt.subscribe("Spa/highrange/set");
      mqtt.subscribe("Spa/jets/1/set");
      mqtt.subscribe("Spa/jets/2/set");
      mqtt.subscribe("Spa/blower/set");
      mqtt.subscribe("Spa/light/set");
      mqtt.subscribe("Spa/relay/1/set");
      mqtt.subscribe("Spa/relay/2/set");

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

  mqtt.publish("Spa/node/debug", topic.c_str());
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
  } else if (topic.equals("Spa/heatingmode/set")) {
    if (payload.equals("ON") && SpaState.restmode == 1) send = 0x51; // ON = Ready; OFF = Rest
    else if (payload.equals("OFF") && SpaState.restmode == 0) send = 0x51;
  } else if (topic.equals("Spa/light/set")) {
    if (payload.equals("ON") && SpaState.light == 0) send = 0x11;
    else if (payload.equals("OFF") && SpaState.light == 1) send = 0x11;
  } else if (topic.equals("Spa/jet/1/set")) {
    if (payload.equals("ON") && SpaState.jet1 == 0) send = 0x04;
    else if (payload.equals("OFF") && SpaState.jet1 == 1) send = 0x04;
  } else if (topic.equals("Spa/jet/2/set")) {
    if (payload.equals("ON") && SpaState.jet2 == 0) send = 0x05;
    else if (payload.equals("OFF") && SpaState.jet2 == 1) send = 0x05;
  } else if (topic.equals("Spa/blower/set")) {
    if (payload.equals("ON") && SpaState.blower == 0) send = 0x0C;
    else if (payload.equals("OFF") && SpaState.blower == 1) send = 0x0C;
  } else if (topic.equals("Spa/highrange/set")) {
    if (payload.equals("ON") && SpaState.highrange == 0) send = 0x50; //ON = High, OFF = Low
    else if (payload.equals("OFF") && SpaState.highrange == 1) send = 0x50;
  } else if (topic.equals("Spa/target_temp/set")) {
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

  // DEBUG:mqtt.publish("Spa/rcv", String(x).c_str()); _yield();

  // Double SOF-marker, drop last one
  if (Q_in[1] == 0x7E && Q_in.size() > 1) Q_in.pop();

  // Complete package
  //if (x == 0x7E && Q_in[0] == 0x7E && Q_in[1] != 0x7E) {
  if (x == 0x7E && Q_in.size() > 2) {
    //print_msg();

    // Unregistered or yet in progress
    if (id == 0) {
      if (Q_in[2] == 0xFE) print_msg(Q_in);

      // FE BF 02:got new client ID
      if (Q_in[2] == 0xFE && Q_in[4] == 0x02) {
        id = Q_in[5];
        if (id > 0x2F) id = 0x2F;

        ID_ack();
        mqtt.publish("Spa/node/id", String(id).c_str());
      }

      // FE BF 00:Any new clients?
      if (Q_in[2] == 0xFE && Q_in[4] == 0x00) {
        ID_request();
      }
    } else if (Q_in[2] == id) {
      if (Q_in[4] == 0x06) {
        // id BF 06:Ready to Send
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
    // FF AF 13:Status Update - Packet index offset 5
    if (Q_in[2] == 0xFF && Q_in[4] == 0x13) {
      if (last_state_crc != Q_in[Q_in[1]]) {
        String s;
        double d = 0.0;
        double c = 0.0;

        // DEBUG for finding meaning:
        //print_msg(Q_in);

        // 25:Flag Byte 20 - Set Temperature
        d = Q_in[25] / 2;
        if (Q_in[25] % 2 == 1) d += 0.5;
        mqtt.publish("Spa/target_temp/state", String(d, 2).c_str());

        // 7:Flag Byte 2 - Actual temperature
        if (Q_in[7] != 0xFF) {
          d = Q_in[7] / 2;
          if (Q_in[7] % 2 == 1) d += 0.5;
          if (c > 0) {
            if ((d > c * 1.2) || (d < c * 0.8)) d = c; //remove spurious readings greater or less than 20% away from previous read
          }

          mqtt.publish("Spa/temperature/state", String(d, 2).c_str());
          c = d;
        } else {
          d = 0;
        }
        // REMARK Move upper publish to HERE to get 0 for unknown temperature

        // 8:Flag Byte 3 Hour & 9:Flag Byte 4 Minute => Time
        if (Q_in[8] < 10) s = "0"; else s = "";
        s = String(Q_in[8]) + ":";
        if (Q_in[9] < 10) s += "0";
        s += String(Q_in[9]);
        mqtt.publish("Spa/time/state", s.c_str());

        // 10:Flag Byte 5 - Heating Mode
        switch (Q_in[10]) {
          case 0:mqtt.publish("Spa/heatingmode/state", STRON); //Ready
          case 3:// Ready-in-Rest
            SpaState.restmode = 0;
            break;
          case 1:mqtt.publish("Spa/heatingmode/state", STROFF); //Rest
            SpaState.restmode = 1;
            break;
        }

        // 15:Flags Byte 10 / Heat status, Temp Range
        d = bitRead(Q_in[15], 4);
        if (d == 0) mqtt.publish("Spa/heatstate/state", STROFF);
        else if (d == 1 || d == 2) mqtt.publish("Spa/heatstate/state", STRON);

        d = bitRead(Q_in[15], 2);
        if (d == 0) {
          mqtt.publish("Spa/highrange/state", STROFF); //LOW
          SpaState.highrange = 0;
        } else if (d == 1) {
          mqtt.publish("Spa/highrange/state", STRON); //HIGH
          SpaState.highrange = 1;
        }

        // 16:Flags Byte 11
        if (bitRead(Q_in[16], 1) == 1) {
          mqtt.publish("Spa/jet/1/state", STRON);
          SpaState.jet1 = 1;
        } else {
          mqtt.publish("Spa/jet/1/state", STROFF);
          SpaState.jet1 = 0;
        }

        if (bitRead(Q_in[16], 3) == 1) {
          mqtt.publish("Spa/jet/2/state", STRON);
          SpaState.jet2 = 1;
        } else {
          mqtt.publish("Spa/jet/2/state", STROFF);
          SpaState.jet2 = 0;
        }

        // 18:Flags Byte 13
        if (bitRead(Q_in[18], 1) == 1)
          mqtt.publish("Spa/circ/state", STRON);
        else
          mqtt.publish("Spa/circ/state", STROFF);

        if (bitRead(Q_in[18], 2) == 1) {
          mqtt.publish("Spa/blower/state", STRON);
          SpaState.blower = 1;
        } else {
          mqtt.publish("Spa/blower/state", STROFF);
          SpaState.blower = 0;
        }
        // 19:Flags Byte 14
        if (Q_in[19] == 0x03) {
          mqtt.publish("Spa/light/state", STRON);
          SpaState.light = 1;
        } else {
          mqtt.publish("Spa/light/state", STROFF);
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
