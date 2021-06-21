#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//const char *ssid = "Taipei RTC ADSL"; // Enter your WiFi name
//const char *password = "0800717718";  // Enter WiFi password

//const char *ssid = "Saturn"; // Enter your WiFi name
//const char *password = "lkjhgfdsa";  // Enter WiFi password

//const char *ssid = "roving1"; // Enter your WiFi name
//const char *password = "rubygirl";  // Enter WiFi password

//const char *ssid = "Mars"; // Enter your WiFi name
const char *ssid = "Patrick-Home"; // Enter your WiFi name
//const char *ssid = "Mars"; // Enter your WiFi name
const char *password = "22302376";  // Enter WiFi password

// MQTT Broker
//const char *mqtt_broker = "s887432.asuscomm.org";
const char *mqtt_broker = "192.168.1.19";
const char *gTopic = "home/study/pcctrl";
const char *mqtt_username = "user";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

typedef enum __CMD_TYPE__ {
  CMD_NONE = 0,
  CMD_LOGIN = 1,
  CMD_LOGOUT
}CMD_TYPE;

#define LOGIN_PIN   5
#define LOGOUT_PIN  4

#define LED_PIN   2

CMD_TYPE gCmdType = CMD_NONE;

int gFirstShut = 1;

void setup() {

   pinMode(LOGIN_PIN, OUTPUT);
   digitalWrite(LOGIN_PIN, HIGH);
   pinMode(LOGOUT_PIN, OUTPUT);
   digitalWrite(LOGOUT_PIN, HIGH);

   pinMode(LED_PIN, OUTPUT);
   digitalWrite(LED_PIN, LOW);
    
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    char id[128];
    client_id.toCharArray(id, client_id.length()+1);
    
    Serial.println("Connecting to public emqx mqtt broker.....");
    if (client.connect(id, mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  
  // publish and subscribe
  //client.publish(topic, "hello emqx");
  client.subscribe(gTopic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic:");
  Serial.println(topic);
  Serial.print("Message:");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  
  Serial.println();
  Serial.println("-----------------------");

  payload[length] = 0;
  String sTopic(topic);
  String cmd((char*)payload);

  Serial.print("CMD:");
  Serial.println(cmd);
  Serial.print("gFirstShut:");
  Serial.println(gFirstShut);
  
  if( sTopic == gTopic && gFirstShut == 0) {
    
    cmd.toUpperCase();
    if( cmd == "LOGIN" ) {
      gCmdType = CMD_LOGIN;
    } else if( cmd == "LOGOUT" ) {
      gCmdType = CMD_LOGOUT;
    } else {
      gCmdType = CMD_NONE;
      Serial.println("Unknown command");
    }
  }

  gFirstShut = 0;
}

void loop() {
  client.loop();

  switch( gCmdType ) {
    case CMD_LOGIN:
      Serial.println("Received LOGIN command");
      digitalWrite(LED_PIN, LOW);

      digitalWrite(LOGIN_PIN, LOW);
      delay(1000);
      digitalWrite(LOGIN_PIN, HIGH);

      gCmdType = CMD_NONE;
      break;
        
    case CMD_LOGOUT:
      Serial.println("Received LOGOUT command");
      digitalWrite(LED_PIN, HIGH);

      digitalWrite(LOGOUT_PIN, LOW);
      delay(1000);
      digitalWrite(LOGOUT_PIN, HIGH);

      gCmdType = CMD_NONE;
      break;

    case CMD_NONE:
    default:
    break;
  }
}
