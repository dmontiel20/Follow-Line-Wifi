#include <WiFi.h> //Wifi library
#include <PubSubClient.h>
#include "pass.h"

// #define EAP_ANONYMOUS_IDENTITY "" // leave as it is
// #define EAP_IDENTITY ""    // Use your URJC email
//#define EAP_PASSWORD ""            // User your URJC password
// #define EAP_USERNAME USER_  // Use your URJC email

//SSID NAME
const char* ssid = "eduroam"; // eduroam SSID

// Configuración del broker MQTT
const char* mqtt_server = "193.147.79.118";  // Dirección del broker MQTT
const int mqtt_port = 21883;                    // Puerto MQTT (por defecto es 1883)
const char* mqtt_topic = "/SETR/2024/11/"; 

WiFiClient espClient;           // Cliente WiFi
PubSubClient client(espClient);

#define RXD2 33
#define TXD2 4

String team_name = "\"MosusduMenorca\"";
String team_id = "\"11\"";

bool exist_data = true;


// Función para reconectar al broker si se pierde la conexión
void reconnect() {
  // Intentar reconectar hasta que se logre
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    // Intentar conectar con un ID de cliente único
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
    } 
    else {
      Serial.print("Error de conexión: ");
      Serial.println(client.state());
      Serial.println("Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

// Function to get the action and the data action and send it to server
void controller_messages(String* buffer){
  int index = buffer->indexOf(':');

  String action = index != -1 ? buffer->substring(0, index) : *buffer;
  String data = index != -1 ? buffer->substring(index + 1) : "";

  Serial.println(action);

  if (action.length() != 1){
    action.remove(action.length() - 1);
  }
  if (action == "I"){
    client.publish(mqtt_topic, (json_message("\"START_LAP\"", -1, -1)).c_str());
  }
  else if (action == "E"){
    client.publish(mqtt_topic, (json_message("\"END_LAP\"", data.toInt(), -1)).c_str());
  }
  else if (action == "O"){
    client.publish(mqtt_topic, (json_message("\"OBSTACLE_DETECTED\"", -1, data.toInt())).c_str());
  }
  else if (action == "L"){
    client.publish(mqtt_topic, (json_message("\"LINE_LOST\"", -1, -1)).c_str());
  }
  else if (action == "P"){
    client.publish(mqtt_topic, (json_message("\"PING\"", data.toInt(), -1)).c_str());
  }
  else if (action == "F"){
    client.publish(mqtt_topic, (json_message("\"LINE_FOUND\"", -1, -1)).c_str());
  }
}

// To meke JSON messages with 3 possibles variables
String json_message(String action, int time, int distant) {
  if (time == -1 && distant == -1) {
    return "{ \"team_name\" : " + String(team_name) + ", \"id\" : " + String(team_id) + ", \"action\" : " + action + "}";
  } 
  else if (time != -1 && distant == -1) { 
    return "{ \"team_name\" : " + String(team_name) + ", \"id\" : " + String(team_id) + ", \"action\" : " + action + ", \"time\" : " + String(time) + "}";
  } 
  else if (time == -1 && distant != -1) { 
    return "{ \"team_name\" : " + String(team_name) + ", \"id\" : " + String(team_id) + ", \"action\" : " + action + ", \"distance\" : " + String(distant) + "}";
  } 
  else {
    return "{ \"team_name\" : " + String(team_name) + ", \"id\" : " + String(team_id) + "}";
  }
}


void setup() {

  // Regular serial connection to show traces for debug porpuses
  Serial.begin(115200);

  // Serial port to communicate with Arduino UNO
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.disconnect(true); 
  WiFi.begin(ssid, EAP_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F(".\n"));
  }
  Serial2.print(WiFi.localIP().toString() + "}");

  // Configurar el servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  // Conectar al broker MQTT
  reconnect();  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Give serial messages sends by arduino
  if (Serial2.available() > 0) {
    String sendBuff = Serial2.readStringUntil('\n');
    
    controller_messages(&sendBuff);
  }
}


