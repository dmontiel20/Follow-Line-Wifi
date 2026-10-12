#include <WiFi.h> //Wifi library
#include <PubSubClient.h>
#include "pass.h"

// #define EAP_ANONYMOUS_IDENTITY "20220719anonymous@urjc.es" // leave as it is
// #define EAP_IDENTITY "d.ponsc.2022@alumnos.urjc.es"    // Use your URJC email
#define EAP_PASSWORD "expiriens"            // User your URJC password
// #define EAP_USERNAME USER_  // Use your URJC email

//SSID NAME
const char* ssid = "Galaxy S10"; // eduroam SSID

// Configuración del broker MQTT
const char* mqtt_server = "193.147.79.118";  // Dirección del broker MQTT
const int mqtt_port = 21883;                    // Puerto MQTT (por defecto es 1883)
const char* mqtt_topic = "/SETR/2024/11/"; 

WiFiClient espClient;           // Cliente WiFi
PubSubClient client(espClient);

#define RXD2 33
#define TXD2 4

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

String sendBuff;

void loop() {

 if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Serial2.available()) {
    String incomingData = "";

    char c = Serial2.read();
    sendBuff += c;
    
    if (c == '$')  {            
      // Serial.print("Received data in serial port from Arduino: ");
      Serial.println(sendBuff);

      client.publish(mqtt_topic, sendBuff.c_str());
      Serial.println("Datos enviados por MQTT.");
      sendBuff = "";
    } 

    // // Leer los datos disponibles
    // while (Serial2.available() > 0) {
    //   char c = Serial2.read();
    //   incomingData += c;
    // }
    // Serial.print(incomingData);

    // Publicar los datos en el tema MQTT
    // if (!incomingData.isEmpty()) {
    //   client.publish(mqtt_topic, sendBuff.c_str());
    //   Serial.println("Datos enviados por MQTT.");
    // }
  }

}
