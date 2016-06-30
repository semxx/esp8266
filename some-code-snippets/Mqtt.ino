#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid =	"IoT";
const char *pass =	"12345678";

IPAddress server(192, 168, 1, 100); // mosquitto address
PubSubClient client(server, 1883);

int counter = 0;
int ms = 0;
int seconds = 0;

char* pubTopic = "/ESP8266/DATA";
char* controlTopic = "/ESP8266/CONTROL/#";
char* statTopic = "/ESP8266/STATUS/GPIO/4";
int GPIO = 4;
String mqtt_user = "test";
String mqtt_pass = "test";
String mqtt_client = "ESP8266";
int heap = 0;

void callback(const MQTT::Publish& sub) {
  Serial.print("Get data from subscribed topic ");
  Serial.print(sub.topic());
  Serial.print(" => ");
  Serial.println(sub.payload_string());
  String expected = "/ESP8266/CONTROL/GPIO/";
  expected += GPIO;
  if (sub.topic() == expected) {
    String s = "ESP8266 GPIO_";
    s += GPIO;
    String stat = "{\"status\":";
    if (sub.payload_string() == "true") {
       digitalWrite(GPIO,0);
       s += " set LOW";
       stat += "\"LOW\"}";
    } else {
       digitalWrite(GPIO,1);
       s += " set HIGH";
       stat += "\"HIGH\"}";
    }
    client.publish(statTopic, stat);
    Serial.println(s);
  }
  // echo
  MQTT::Publish newpub(pubTopic, sub.payload(), sub.payload_len());
  client.publish(newpub);
}

void restart() {
    Serial.println("Will reset and try again...");
    abort();
}
void setup()
{
  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  pinMode(GPIO, OUTPUT);
  digitalWrite(GPIO,1);

  client.set_callback(callback);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.printDiag(Serial);
  } else {
    restart();
  }

  Serial.println("Connecting to MQTT broker ");
  if (client.connect(MQTT::Connect(mqtt_client).set_auth(mqtt_user, mqtt_pass))) {
    Serial.println("Connected to MQTT broker");
    client.subscribe(controlTopic);
  } else {
    restart();
  }
  heap = ESP.getFreeHeap();
}

void loop()
{
  if ( micros()/1000 - ms  > 1000 || ms > micros()/1000 ) {
     Serial.print("HEAP: ");
     Serial.println(ESP.getFreeHeap());
     if ( heap > ESP.getFreeHeap() ) {
       Serial.println("Detect mem leak!");
       heap = ESP.getFreeHeap();
     }
     ms = micros()/1000;
     ++seconds;
  }
  if ( seconds >= 10 ) {
    seconds = 0;
    ++counter;
    String payload = "{\"micros\":";
    payload += ms;
    payload += ",\"counter\":";
    payload += counter;
    payload += "}";
  
    if (client.connected()){
      Serial.print("Sending payload: ");
      Serial.print(payload);
      if (client.publish(pubTopic, (char*) payload.c_str())) {
        Serial.println(" Publish OK");
      } else {
        restart();
      }
    } else {
      restart();
    }
  }
  if ( !client.loop() )  {
    restart();
  }
}

