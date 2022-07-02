// This #include statement was automatically added by the Particle IDE.
#include <HC_SR04.h>


// -----------------------------------
// Garage Remote
// Last Updated Date: 12/10/2021
// Revisions
// * Removed Magswitch, added distance sensor, added library, added pushover integration
// * Notification Timing redo
// * Added PIR Sensor
// * Remove PIR sensor Logic (05-06-2021)
// * Fix Compiling Errors
// Installed: YES
// Installed Date: 03/22/2018
// Installed on Photon 'Garage' 
// -----------------------------------

int led1 = D0;
int Trig = D4;   // HC-SR04 Trigger
int Echo = D5;   // HC-SR04 Echo
int PIR = D3;    // PIR SENSOR "OUT" PIN
int Relay = D6;  // RELAY
bool doorOpen = false;
bool pushNotif = false;
int pirState = LOW; 
int PIRCount = 0;
String doorMessage = "";

// HC_SR04 Related
HC_SR04 doorDistance = HC_SR04(Trig, Echo, 50.0, 300.0);
double cm = 0.0;
bool snooze = false;  // false = disable, true =  enable

// Door Sensor Time Related
unsigned long doorSensorlastTime = 0UL;
unsigned long eventCheckInterval = 1000UL * 5UL; // Time in seconds, how often we check
unsigned long delayedNotificationTime = 1000UL * 60UL * 10UL; // Pushover notification frequency Time in Minutes

unsigned long _firstEventTime; // When the first notification is sent
bool delayNotification = false;
int notificationCounter = 0;

// PIR
int triggeredStatus = 0; // 0: not triggered, lights are off / 1: triggered, Lights are On
unsigned long LightServiceTime = 1000UL * 60UL * 15UL; // Ideally you want the light ON for about 5-10 min, then turn it off if no events are detected. 5UL = 5 MIN
unsigned long PIRChecklastTime = 0UL;
unsigned long PIRSensorCheckInterval = 1000UL * 1UL; // Time in seconds, how often we check, e.g.: 1 second.
bool lightStatus = false;
unsigned long pirSensorlastTime = 0UL;

// Reading data to String
char cmString[40];
char myIpString[40];
int rssi;

unsigned long pirSensorNow = 0UL;
unsigned long doorSensorNow = 0UL;

void setup()
{
    //particle.publish("smart-home", "Running Setup", 60, PRIVATE);
    STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
    
    // Serial.begin(9600);
    //WiFi.selectAntenna(ANT_INTERNAL);
   
   // Here's the pin configuration, same as last time
   pinMode(led1, OUTPUT);
   pinMode(Trig, OUTPUT);
   pinMode(Echo, INPUT);
   pinMode(PIR, INPUT);
   pinMode(Relay, OUTPUT);
   
   // HC_SR04 Related
   Particle.variable("cm", &cm, DOUBLE);
   Particle.variable("IPAddress", myIpString, STRING);
   Particle.variable("RSSI", &rssi, INT);
   
    //Particle.variable("inches", &inches, DOUBLE);

   // Declare Particle Function
   Particle.function("OpenGarage",OpenGarage);
   Particle.function("Snooze",Snooze);
   Particle.function("TestPush",TestPush);
   Particle.function("PIRSensorStatus",PIRSensorStatus);
   Particle.function("GetAvrgDist",GetAvrgDist);
   Particle.function("LightStatus",LightStatus);
   Particle.function("ParticleDoorStatus",ParticleDoorStatus);
   Particle.function("TurnLights",TurnLights);
   Particle.function("GetTPLinkToken",GetTPLinkToken);

   // Initialize 
   digitalWrite(led1, LOW);
   bool snooze = false;  // false = disable, true =  enable


   //digitalWrite(Relay, HIGH);
   
   IPAddress myIP = WiFi.localIP();
   rssi = WiFi.RSSI();
   sprintf(myIpString, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
   
//   snooze = true;
   
   if (!snooze) {
     Particle.publish("smart-home", "Garage Restarted "  + String(myIpString) + " " + String(rssi) + " Ver 20220701", 60, PRIVATE);
    //  Particle.publish("smart-home", "IP: " + String(myIpString), 60, PRIVATE);
    //  Particle.publish("smart-home", "RSSI: " + String(rssi), 60, PRIVATE);
   }
   
   // subacribe to event
   Particle.subscribe("hook-response/TpLInkToken", myHandler, MY_DEVICES);
   Particle.subscribe("hook-response/tplink_cloud", myHandler, MY_DEVICES);
   

}
 

void loop()
{
    rssi = WiFi.RSSI();
    
    if (doorOpen){
        checkDelayNotification(_firstEventTime, delayedNotificationTime);
    }
    
    // Check every second PIRSensorCheckInterval
    // If detected, turn light, set activeTimeWait true, wait for x seconds (LightServiceTime, e.g.:5min)
    
    pirSensorNow = millis();
    
    //  if ((pirSensorNow - pirSensorlastTime > PIRSensorCheckInterval))
    // {
    //     pirSensorlastTime =  pirSensorNow;
    //     if (PIRSensorStatus("x") == 1) {
            
    //         if (!lightStatus)
    //         {
    //             if (!snooze)
    //                 Particle.publish("smart-home", "Movement Detected in Garage", 60, PRIVATE);
    //             lightStatus = true;
    //             triggeredStatus = 1;
    //             // Particle.publish("tplink_cloud","1", PRIVATE); Turn Lights on #fixThis
    //             delay(20000);
    //         }else {
    //             //Particle.publish("kasa-outlet", "...", PRIVATE);
    //             //Particle.publish("pir-sensor", "Turn OFF Lights in Garage", 20, PUBLIC);'
    //             //Particle.publish("smart-home", "Garage Lights off", 60, PRIVATE);
    //             lightStatus = false;
    //             triggeredStatus = 0;
    //             // Particle.publish("tplink_cloud","0", PRIVATE);  Turn Lights off #fixThis
    //             delay(20000);
    //         }

    //     }
      
    // }
  
    // MOTION DETECTION
    // if (PIRSensorStatus("x") == 1) // 1 = Motion Detected
    // {
    //     Particle.publish("pir-sensor", "X_Lamp Triggered", 60, PRIVATE);
    // }
    
    doorSensorNow = millis();

    if ((doorSensorNow - doorSensorlastTime > eventCheckInterval) && !delayNotification)
    {
        doorSensorlastTime =  doorSensorNow;
        //cm = doorDistance.getDistanceCM();
        //use avrg instead, "cm" value does nothing
        cm = GetAvrgDist("cm");
        
        // DOOR STATUS
        // When door is closed
        if (cm >= 260 && doorOpen && cm !=-1){
            
            sprintf(cmString,"%.2f",cm);
            Particle.publish("Distance_Close",cmString);
            Particle.publish("garage_door", "Garage Door is now Closed", 60, PRIVATE);
            Particle.publish("smart-home", "Garage Door is now Closed", 60, PRIVATE);
            notificationCounter = 0;
            doorOpen = false;
            snooze = false;
           
        // When door is open
        } else if (cm < 70 && cm > 40 && cm !=-1 && !snooze ){
            // Particle Notification
             
               
            // if (getDoorStatus() == 1) {
            //   // Allow one notification then wait 5 minutes
            //   if (notificationCounter == 0){
            //     _firstEventTime = millis();
            //      delayNotification = true;
            //      notificationCounter++ ; 
            //       //Particle.publish("smart-home", "Garage just opened", 60, PRIVATE);
            //     //   Particle.publish("garage_door", "Garage just opened", 60, PRIVATE);
            //       doorOpen =  true;
            //   } 
              if (!delayNotification)
               {
                   delayNotification = true;
                   _firstEventTime = millis();
                   doorMessage = "Garage Door is Open";
                  Particle.publish("garage_door", doorMessage, 60, PRIVATE);
                  Particle.publish("smart-home", doorMessage, 60, PRIVATE);
                  
                  sprintf(cmString,"%.2f",cm);
                  doorMessage = "Distance Open: ";
                  Particle.publish("garage_door",cmString,60, PRIVATE);
                  //Particle.publish("smart-home" ,cmString,60, PRIVATE);
                  
                   doorOpen =  true;
               }
            // }
        }
        // Testing Push Notification
        if (pushNotif)
        {
            if(!delayNotification && notificationCounter==0)
            {
                doorMessage = "Garage just opened";
                Particle.publish("garage_door", doorMessage, 60, PRIVATE);
                Particle.publish("smart-home", doorMessage, 60, PRIVATE);
                doorOpen =  true;
                _firstEventTime = millis();
                delayNotification = true;
                notificationCounter++; 
            } else {
                delayNotification = false;
                doorMessage = "Reminder: Garage Door is Open";
                Particle.publish("smart-home", doorMessage, 60, PRIVATE);
                doorOpen =  true;
            }
             
        } 
    }
    
   
}

int Snooze(String val){
    
    if(snooze) {
        snooze = false;
        Particle.publish("smart-home", "Notifications are enable", 60, PRIVATE);
        return 2;  
    } else {
        snooze = false; // temp fix
        Particle.publish("smart-home", "Notifications are disabled", 60, PRIVATE);
        return 1;  
    }
}

int ParticleDoorStatus(String Val)
{
    return getDoorStatus();
}

int getDoorStatus() {
    // 1 - Opening
    // 2 - Close
    // 3 - Undefined
    
    //Variables
    double firstReading;
    double secondReading;
    
    // Take Readings
    firstReading = doorDistance.getDistanceCM();
    delay(3000);
    secondReading = doorDistance.getDistanceCM();
    
    if (secondReading < 70 && firstReading  < 70) {
        return 1;
    } else {
        return 2;
    }
}

// PARTICLE FUNCTION
int LightStatus(String val)
{
    return triggeredStatus;
}

// PARTICLE FUNCTION
int TurnLights(String val)
{
    if (!lightStatus)
            {
                //Particle.publish("smart-home", "Turn Lights off", 60, PRIVATE);
                lightStatus = true;
                triggeredStatus = 1;
                Particle.publish("tplink_cloud","1", PRIVATE); 
                delay(20000);
                return 1;
            }else {
                //Particle.publish("smart-home", "Turn Lights off", 60, PRIVATE);
                lightStatus = false;
                triggeredStatus = 0;
                Particle.publish("tplink_cloud","0", PRIVATE);
                delay(20000);
                return 0;
            }
}

int GetTPLinkToken(String val)
{
   Particle.publish("TpLInkToken","1", PRIVATE); 
   return 77;
}

// PARTICLE FUNCTION
int PIRSensorStatus(String val)
{
    // Particle.publish("pir-sensor", "F_Motion Detected in Garage", 60, PRIVATE);
    // Particle.publish("pir-sensor", "F_Turning On Light", 60, PRIVATE);
    // return getPIRSensorStatus(PIR);
    unsigned long PIRnow = millis();
    
    if (triggeredStatus == 0)
    {
        if (getPIRSensorStatus(PIR) == 1){
            Particle.publish("pir-sensor", "triggeredStatus 1", 60, PRIVATE);
            triggeredStatus = 1;
            PIRChecklastTime = PIRnow;
            return 1;
        } else {
            return 0;
        }
            
    }
    
    // Trigered Status (Light is on)
    if (triggeredStatus == 1)
    {
        if ((PIRnow - PIRChecklastTime > LightServiceTime))
        {
                PIRChecklastTime = PIRnow;
                Particle.publish("pir-sensor", "Checking Room", 60, PRIVATE);
                if (getPIRSensorStatus(PIR) == 1)
                 return 1;
        } else {
            return 0;
        }
    }
    // Non Trigered Status
    return 0;

}

int getPIRSensorStatus(int PIN) {
    // 1 - Detected Presence
    // 2 - No events (Nothing Detected)
    // 3 - Undefined
  
  int val = 0;
  
  val = digitalRead(PIN);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      pirState = HIGH;
       //Particle.publish("pir-sensor", "HIGH", 60, PRIVATE);
      return 1;
    } else 
     return 0;
  } 
  if (val == LOW) {
    if (pirState == HIGH){
      pirState = LOW;
      //Particle.publish("pir-sensor", "LOW", 60, PRIVATE);
      return 1;
    } else 
     return 0;
  }
  return 0;
}

// PARTICLE FUNCTION
int GetAvrgDist(String val) {
    //return doorDistance.getDistanceCM();
    //HC_SR04 dist = HC_SR04(Trig, Echo, 50.0, 300.0);
    double sumDist = 0;
    int a;
    for( a = 1; a <=10 ; a++){
      sumDist = sumDist + doorDistance.getDistanceCM();
    }
    return sumDist/10;
}

int OpenGarage(String command) {

    if (command=="on") {
        
        //digitalWrite(led1,HIGH);  // not in use
        digitalWrite(Relay,HIGH); // Send Relay signal
        delay(500);
        digitalWrite(Relay,LOW);
        
       Particle.publish("garage_door", "command on"); 
       
       if (doorOpen)
       {
           doorMessage = "Garage is Closing";
           Particle.publish("garage_door",doorMessage);
           Particle.publish("smart-home", doorMessage, 60, PRIVATE);
           lightStatus = false;
           delayNotification = false;
           doorOpen = false;
           //Particle.publish("tplink_cloud","0", PRIVATE);
           //delay(30000);
       }
       else
       {
           doorMessage = "Garage is Opening";
           Particle.publish("garage_door",doorMessage); 
           Particle.publish("smart-home", doorMessage, 60, PRIVATE);
           
           lightStatus = true;
           //Particle.publish("tplink_cloud","1", PRIVATE);
           //delay(30000);
           doorOpen = true;
       }
        
        // Particle.publish("smart-home",doorMessage, 60, PRIVATE);
        // Particle.publish("garage_door",doorMessage); 
        //delay(500);
        
        //if (digitalRead(Relay)){
           //digitalWrite(led1,LOW);
          // digitalWrite(Relay,LOW);
        //}
        return 1;
    }
    else if (command=="off") {
        digitalWrite(led1,LOW);
        return 0;
    }
    else {
        return -1;
    }
}

int TestPush(String val) {
    // 1: Test Push notification
    // 2: Normal (no test notification)
    if (val == "1") {
        pushNotif = true;
        Particle.publish("smart-home", "Test Push Notification Value 1", 60, PRIVATE);
        return 1;
    } 
    if (val == "0") {
        pushNotif = false;
        Particle.publish("smart-home", "Test Push Notification Value 0", 60, PRIVATE);
        return 0;
    }
    return 0;
}

bool checkDelayNotification(unsigned long firstEventTime, unsigned long eventDelayTime) {
    unsigned long now = millis();
    if (now - firstEventTime > eventDelayTime)
    delayNotification = false; //enable inmediate notificiation 
    return delayNotification;
}

// handler
int i = 0;
void myHandler(const char *event, const char *data)
{
//   i++;
//   Serial.print(i);
//   Serial.print(event);
//   Serial.print(", data: ");
  if (data)
   {
       Particle.publish("debug", "Hook Response", 60, PRIVATE);
       Particle.publish("debug", String(data), 60, PRIVATE);
    //   Particle.publish("debug", event, 60, PRIVATE);
   }
    // Serial.println(data);
  else
    Particle.publish("debug", "No data", 60, PRIVATE);
}





