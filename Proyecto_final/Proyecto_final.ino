#include <WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h>


/*Configuración de Wifi*/
const char* SSID     = "Cyber Paparrines"; // Your ssid
const char* WIFI_PASS = "TAXITODIC20"; // Your WIFI_PASS 
/*Configuración MQTT*/
const char* CLIENT_ID ="esp32";
const char* DNS_SERVER = "ec2-54-177-96-211.us-west-1.compute.amazonaws.com";
const char* MQ_USER = "esp32";
const char* MQ_PASSWORD = "12345678";
/*Aqui se configuran los pines donde debemos conectar el sensor*/
#define TRIGGER_PIN  18
#define ECHO_PIN     19
#define MAX_DISTANCE 20
/*Led que indica si hay conexión MQTT*/
const int LED_MQTT = 2;
/*Variables para indicar el conteo*/
int cm = 0, cmAnterior = 0, contador = 0;
/*Crear el objeto de la clase NewPing*/
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

WiFiClient espClient;
PubSubClient client(espClient);
 
void setup_wifi() {
  Serial.println("Inician conexión con red: " + (String) SSID);
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conexión a Wifi satisfactoria !!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback ...");
  Serial.println(topic);
  if(String(topic) == "LINEA1/esp32/CONTADOR/RESET"){
    contador = 0;
    Serial.println("Contador reiniciado...");
  }
  
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intento de conexión MQTT ...");
    if (client.connect(CLIENT_ID, MQ_USER, MQ_PASSWORD)) {
      digitalWrite(LED_MQTT, HIGH);
      Serial.println("Cliente conectado ...");
      client.subscribe("LINEA1/esp32/CONTADOR/RESET");
    } else {
      digitalWrite(LED_MQTT, LOW);
      Serial.print("Fallo al intentar realizar la conexión, código de error (rc)= ");
      Serial.print(client.state());
      Serial.println("Intento de reconexión después de 5 segundos");
      delay(5000);}
    }      
}

void setup() {
  pinMode(LED_MQTT  , OUTPUT);    
  Serial.begin(115200);
  setup_wifi();
  client.setServer(DNS_SERVER, 4001);
  client.setCallback(callback);
}

void loop() {
  // Obtener medicion de tiempo de viaje del sonido y guardar en variable uS
  int uS = sonar.ping_median(); 
  String pConteo = "", msg = "";
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  cm = uS / US_ROUNDTRIP_CM;
  
  if( cmAnterior != 0 && cm == 0){
    contador ++;
  }
  cmAnterior = cm;
  Serial.println("Contador = "  + (String) contador);

  char message[56];
  msg = "{\"contador\":  " + (String) contador + "}";  
  sprintf(message, "%s", + "{\"contador\":  " + (String) contador + "}");

  msg.toCharArray(message, 56);
  
  client.publish( "LINEA1/esp32/CONTADOR/CONTEO", message, 2);
  
  delay(1000);
}
