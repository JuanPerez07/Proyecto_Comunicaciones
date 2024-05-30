#include "config.h" 
// configuración pantalla OLED
const int oledAddress = 0x3C;  // La dirección I2C de la pantalla OLED
const int sdaPin = 5;  // Pin SDA, puede variar según tu ESP32
const int sclPin = 4;  // Pin SCL, puede variar según tu ESP32
// Crea un objeto display
SSD1306Wire display(oledAddress, sdaPin, sclPin);
// Variables de estado de conexión
bool internetOk = false;
bool brokerOk = false;
bool sensorOk = false;
// Configuración de username y password del WiFi
//    const char* ssid;
//    const char* password;
// Configuración de la dirección y puerto del broker
//    const char* server_mqtt;
//    const int puerto_mqtt;
// configuración de username y password del cliente 
//    const char* mqtt_username;
//    const char* mqtt_password;
// Configuración del certificado de raíz: token necesario para la seguridad TLS
//    static const char* root_ca;
WiFiClientSecure ClientEsp;
PubSubClient client(ClientEsp);
static float temp = 0;
static float hum = 0;
String _topic;
String _payload;
// sensor GPIO signal and type of sensor 
#define PIN_SIGNAL 25  
DHT dht(PIN_SIGNAL, DHT11);
// function to read values from the sensor each 5 seconds
void readSensor(){
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  if (isnan(hum) || isnan(temp)) {
    sensorOk = false;
  } else {
    sensorOk = true;
  }
  delay(5000);
}
// function to connect to the WiFi
void wificonf(){
  delay(10);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
  }
  internetOk = true;
  delay(500);
}
// function to connect to the HiveMQ borker
void connectMQTT(){
  while (!client.connected()){
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)){
      brokerOk = true;
    } else{ // connection failure
      Serial.print("Falla, estado= ");
      Serial.println(client.state());
      delay(5000); 
    }
  }
}
// function to send the callback to the broker
void callback(char* topic, byte* payload, unsigned int length){
  String conc_payload_;
  for (int i=0;i<length;i++){
    conc_payload_ +=(char)payload[i];
  }
  _topic=topic;
  _payload=conc_payload_;
}

void setup(){
  ClientEsp.setCACert(root_ca);  // Set the root certificate
  Serial.begin(115200);
  // Configure WiFi, MQTT broker direction and callback
  wificonf();
  client.setServer(server_mqtt,puerto_mqtt);
  client.setCallback(callback);
  // setup the humidity and temperature sensor
  pinMode(PIN_SIGNAL, INPUT_PULLUP); // input pin with Rpull-up
  dht.begin();  // initialize the object of the sensor
  // screen OLED
  display.init();  // initialize screen
  display.flipScreenVertically();  // rotate screen to display horizontally
  display.setFont(ArialMT_Plain_10); 
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
}

void loop(){
  if (!client.connected()){
    connectMQTT();
  }
  client.loop();
  readSensor(); // reads values for temperature and humidity
  //----------------------------------------------------------------------------
  // show connection state on the screen OLED
  if (internetOk) {
    display.drawString(display.getWidth() / 2, 13, "Connected to WiFi");
  }else{
    display.drawString(display.getWidth() / 2, 13, "Connecting to WiFi...");
  }
  if (brokerOk) {
    display.drawString(display.getWidth() / 2, 22, "Connected to broker");
  }else{
    display.drawString(display.getWidth() / 2, 22, "Connecting to broker...");
  }
  if (sensorOk) {
    display.drawString(display.getWidth() / 2, 31, "Connected to DHT11");
  }else{
    display.drawString(display.getWidth() / 2, 31, "DHT11 connection lost");
  }
  display.display();  // show messages in the screen OLED
  //--------------------------------------------------------------
  // publish to the broker in the cloud
  if(not isnan(hum) && not isnan(temp)){
    client.publish("temperature", String(temp).c_str());
    client.publish("humidity", String(hum).c_str());
  }
}






