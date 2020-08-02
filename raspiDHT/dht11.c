#include <wiringPi.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>


typedef unsigned char uint8;  
typedef unsigned int  uint16;  
typedef unsigned long uint32;  
  
#define HIGH_TIME 32  
#define ADDRESS     "tcp://192.168.0.8:1883"
#define CLIENTID    "dht11"
#define TOPIC_TEMPERATURE  "home/dht11/temperature"
#define TOPIC_HUMIDITY "home/dht11/humidity"
#define QOS         1
#define TIMEOUT     10000L  
 
int pinNumber = 0; 
uint32 databuf;  
MQTTClient client;  
  
uint8 readSensorData(void)  
{  
    uint8 crc;   
    uint8 i;  
   
    pinMode(pinNumber, OUTPUT); // set mode to output  
    digitalWrite(pinNumber, 0); // output a high level   
    delay(25);  
    digitalWrite(pinNumber, 1); // output a low level   
    pinMode(pinNumber, INPUT); // set mode to input  
    pullUpDnControl(pinNumber, PUD_UP);  
  
    delayMicroseconds(27);  
    if(digitalRead(pinNumber) == 0) //SENSOR ANS  
    {  
        while(!digitalRead(pinNumber)); //wait to high  
		for(i=0;i<32;i++)  
		{  
		   while(digitalRead(pinNumber)); //data clock start  
		   while(!digitalRead(pinNumber)); //data start 
		   delayMicroseconds(HIGH_TIME);
		   databuf*=2;  
           if(digitalRead(pinNumber)==1) //1  
           {
			   databuf++;  
           }  
        }
		for(i=0;i<8;i++)  
        {  
		   while(digitalRead(pinNumber)); //data clock start  
		   while(!digitalRead(pinNumber)); //data start
		   delayMicroseconds(HIGH_TIME);
		   crc*=2;
		   if(digitalRead(pinNumber)==1) //1  
           {  
                crc++;  
           }  
        }  
		return 1;  
    }
	else  
    {
		return 0;
	}  
}  

void setupMQTTBroker(){
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    printf("%d \n", rc);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    } 
} 

int msgID = 0;
void mqttPublish(char* topic, float payload){
      MQTTClient_deliveryToken token;
      MQTTClient_message pubmsg = MQTTClient_message_initializer;
      char stringPayload[10];
      gcvt(payload, 4, stringPayload);
      pubmsg.payload = stringPayload;
      pubmsg.payloadlen = (int)strlen(stringPayload)+1;
      pubmsg.qos = 1;
      int rc;
      rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
      printf("%d", rc);
      printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), stringPayload, topic, CLIENTID);
      rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
      printf("Message with delivery token %d delivered. RC %d\n", token,rc); 
}  

int main (void)  
{  
  
  printf("Use GPIO1 to read data!\n");  
  
  if (-1 == wiringPiSetup()) 
  {  
    printf("Setup wiringPi failed!");  
    return -1;  
  }  
   
  pinMode(pinNumber, OUTPUT); // set mode to output  
  digitalWrite(pinNumber, 1); // output a high level   
  
  printf("Enter OS-------\n"); 
  setupMQTTBroker(); 
  while(1) 
  {
	  pinMode(pinNumber,OUTPUT); // set mode to output
	  digitalWrite(pinNumber, 1); // output a high level
	  delay(5000);
	  if(readSensorData())
	  {
      float temp = (databuf>>8)&0xff;
      temp += (databuf&0xff)/100.0f;

      float hum = (databuf>>24)&0xff;
      hum += ((databuf>>16)&0xff)/100.0f; 

		  printf("Congratulations ! Sensor data read ok!\n"); 
		  printf("RH:%2.2f%\n",hum);
		  printf("TMP:%2.1fC\n",temp);
      mqttPublish(TOPIC_TEMPERATURE, temp);
      mqttPublish(TOPIC_HUMIDITY, hum); 
      databuf = 0;
	 }  
    else  
    {  
       printf("Sorry! Sensor dosent ans!\n");  
       databuf = 0;  
    }  
  }  
  return 0;  
} 
