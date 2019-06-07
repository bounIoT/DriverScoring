#include <SPI.h>
#include <YunClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#define MS_PROXY "quickstart.messaging.internetofthings.ibmcloud.com"
#define MQTT_CLIENTID "d:quickstart:iotsample-arduino:90A2DAF8157E"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_PORT 1883
#define MQTT_MAX_PACKET_SIZE 100
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
double x_cal=0;
double y_cal=0;
double z_cal=0;
double x;
double y;
double z;
double score;

YunClient c;
IPStack ipstack(c);

MQTT::Client<IPStack, Countdown, 100, 1> client = MQTT::Client<IPStack, Countdown, 100, 1>(ipstack);

String deviceEvent;


void setup() {
  Bridge.begin();
  Console.begin();
  accel.begin();
  calibrate();
  delay(2000);
}


void loop() {
  
  int rc = -1;
  if (!client.isConnected()) {
    Serial.println("Connecting to IoT Foundation");
    rc = ipstack.connect(( (char *) MS_PROXY), MQTT_PORT);
    
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char *)MQTT_CLIENTID;    
    rc = -1;
    while ((rc = client.connect(data)) != 0)
    ;
    Serial.println("Connected successfully\n");
   
    Serial.println("____________________________________________________________________________");
  }

  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  
  char X[60];
  char Y[60];
  char Z[60];
  char Sc[60];

  x=getX();
  y=getY();
  z=getZ();
  score=formula(x,y,z);
  dtostrf(x,sizeof(x),3, X);
  dtostrf(y,sizeof(y),3, Y);
  dtostrf(z,sizeof(z),3, Z);
  dtostrf(score,sizeof(score),3, Sc);


  String str="{\"d\":{\"X\":";
  str+= X;
  str+=",";
  str+= String("\"Y\":");
  str += Y;
  str+=",";
  str += String("\"Z\":");
  str += Z;
  str+=",";
  str += String("\"Score\":");
  str += Sc;
  str += "}}";  
  
  deviceEvent =str;
  
  Serial.print("\t");
  Serial.print(X);
  Serial.print("\t");
  Serial.print(Y);
  Serial.print("\t");
  Serial.print(Z);
  Serial.print("\t");
  Serial.print(Sc);
  Serial.print("\t\t");
  deviceEvent.toCharArray(X, 60);
  Serial.println(X);
  message.payload = X;
  message.payloadlen = strlen(X);
  rc = client.publish(MQTT_TOPIC, message);
  message.payload = Y;
  message.payloadlen = strlen(Y);
  rc = client.publish(MQTT_TOPIC, message);
  message.payload = Z;
  message.payloadlen = strlen(Z);
  rc = client.publish(MQTT_TOPIC, message);
  
  message.payload = Sc;
  message.payloadlen = strlen(Sc);
  rc = client.publish(MQTT_TOPIC, message);


  if (rc != 0) {
    Serial.print("return code from publish was ");
    Serial.println(rc);
  }
  client.yield(1000);
}
double getX(void){
sensors_event_t event;
 accel.getEvent(&event);
  
 x=accel.raw.x;
 x-=x_cal;
 x/=z_cal;
 return x; 
  
 }

double getY(void){
 sensors_event_t event;
 accel.getEvent(&event);
  
  y=accel.raw.y; 
  y-=y_cal;
  
  y/=z_cal;
  return y;
 }
double getZ(void){
   sensors_event_t event;
 accel.getEvent(&event);
   z=accel.raw.z;
  z/=z_cal;
  return z;  
}

double formula(double x,double y,double z){
  

 // double Nt=4*z+2*sqrt(y*y+(z-1)*(z-1))+4*x;
  double Nz=3*sqrt(16*x*x+4*y*y+(z-1)*(z-1))+5*y;

  return 15*Nz;
 }
void calibrate(void){
  sensors_event_t event;
  accel.getEvent(&event);
  double x[10];
  double y[10];
  double z[10];


  for(int i=0;i<10;i++){
    x[i]=accel.raw.x;
    y[i]=accel.raw.y;
    z[i]=accel.raw.z;
  Serial.print("X: ");
  Serial.print(x[i]);
  Serial.print(" Y: ");
  Serial.print(y[i]);
  Serial.print(" Z: ");
  Serial.println(z[i]);

  }
  for(int i=0;i<10;i++){
    x_cal+=x[i];
    y_cal+=y[i];
    z_cal+=z[i];
  }
  x_cal/=10;
  y_cal/=10;
  z_cal/=10;
}
