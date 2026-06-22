#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
const int rs =8, en =9, d4 =10, d5 =11, d6 =12, d7 =13;
#include <LiquidCrystal.h>
int buz=7;
int ss=A0;
int sval;
int pb=3;
int temperatureF;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int spo2=0,ssum=0,hsum=0;
int cnt=0;
//float temperatureF;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
int kk=0;
int ab=0;
void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(buz,OUTPUT);
  pinMode(pb,INPUT);
  pinMode(ss,INPUT);
  digitalWrite(buz,1);
  lcd.print("   WELCOME");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY();
  delay(2000);
}

void loop()
{
  long irValue = particleSensor.getIR();
   
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  
  float temperatureF = particleSensor.readTemperatureF()-6; //Because I am a bad global citizen
  beatAvg=beatsPerMinute;
  if(beatAvg<40)
  spo2=0;
  else if(beatAvg>140)
  spo2=map(beatAvg,140,255,80,50);
  else if(beatAvg>=40 && beatAvg<60)
  spo2=map(beatAvg,40,60,70,100);
  else
  spo2=map(beatAvg,60,140,100,80);

  ssum=ssum+spo2;
  hsum=hsum+beatAvg;
  cnt=cnt+1;
  if(cnt==10)
  {
    cnt=0;
    spo2=ssum/10;
    beatAvg=hsum/10;
ssum=0;
hsum=0;
if (irValue < 50000)
  {
   
  lcd.clear();
        lcd.print(" No finger?");
 // Serial.println(" No finger?");

  beatAvg=0;
  spo2=0;
  temperatureF=0;
  }
  
else
  { 
  if(beatAvg>40)
  {
        int sval=digitalRead(ss);
        int pval=digitalRead(pb);
        lcd.clear();
        lcd.print("H:"+ String(beatAvg) + " S:"+ String(spo2)  + "  P:"+ String(pval));
        lcd.setCursor(0,1);
        lcd.print("t:"+ String(temperatureF) +  "  S:"+ String(sval));

        if((spo2>30 && spo2<70) || beatAvg>100|| beatAvg<30 || temperatureF>100 || sval==1 || pval==0)
        {
          ab=ab+1;
          if(ab>4 && kk==0)
          {
            send_sms();
            Serial.println("Caling..      ");
            Serial.println("AT");
            delay(1000);
            Serial.println("AT+CMGF=1");
            delay(2000);
            Serial.println("ATDXXXXXXXXXX;");                                                              ;");
            delay(20000);
            kk=1;
          }
        }
        else
        {
          ab=0;
          kk=0;
        }
  
  }
  else
  {
lcd.clear();
        lcd.print("Reading..");
  beatAvg=0;
  spo2=0;
  temperatureF=0;
  }
  }
  }
  }  
 void send_sms()
{
//digitalWrite(buz,1);
Serial.println("Sending SMS...");
delay(1000);
Serial.println("AT");    
delay(1000);  
Serial.println("ATE0");    
delay(1000);  
Serial.println("AT+CMGF=1");    
delay(1000);  
Serial.print("AT+CMGS=\"XXXXXXXXXX\"\r\n");
delay(1000);
Serial.println("Alert - Abnormal Condition : Please check Sensor values:");
Serial.println("HB:"+ String(beatAvg));
Serial.println("SPO2:"+ String(spo2));
Serial.println("TEMP:"+ String(temperatureF));
Serial.println("Sound:"+ String(sval));
delay(100);
Serial.println((char)26);// ASCII code of CTRL+Z
delay(2000);
Serial.println("Sending SMS...");
delay(1000);
Serial.println("AT");    
delay(1000);  
Serial.println("ATE0");    
delay(1000);  
Serial.println("AT+CMGF=1");    
delay(1000);  
Serial.print("AT+CMGS=\"XXXXXXXXXX\"\r\n");
  delay(1000);
Serial.println("Alert - Abnormal Condition : Please check Sensor values:");
Serial.println("HB:"+ String(beatAvg));
Serial.println("SPO2:"+String(spo2));
Serial.println("TEMP:"+String(temperatureF));
Serial.println("Sound:"+String(sval));
delay(100);
Serial.println((char)26);// ASCII code of CTRL+Z
delay(2000);
//digitalWrite(buz,0);
}
