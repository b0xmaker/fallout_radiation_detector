#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// This script was made for the LOLIN32 (v1) board, not the newer ones. Modification may be required for different ESP32 boards.
// It has been modified from source to fix the breaking API changes that have been published in later ESP32 SDKs since original release.

TaskHandle_t Task1;

//const int LED_BUILTIN = 2;
//const int beepPin = 0;
//const int flashPin = 12;
//const int powerLEDPin = 18;  

// Attach a 10-segment LED bar graph
// Each segment indicates the current radiation level

// Pins for the 10-segment LED bar graph
int radiationPins[] = {15, 23, 4, 5, 18, 19, 21, 16, 2, 10};



// Threshold for each level
int radiationThresholds[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 100};

// radiation value holds total accumulated radiation user has experienced
int radiationValue = 0;

// radiation level increases when the radiation level is above the threshold
int radiationLevel = 0;

int beepFlashDelay = 0;

bool failedScan = true;
int missed = 0;
String knownBLEAddresses[] = {"80:e1:26:6a:46:56"};
const int RSSI_THRESHOLD = -70;
bool device_found;
const int scanTime = 1; //In seconds
int numberOfDevices = 0;
int rssi = 0;
int lowestRssi = -1000;
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      for (int i = 0; i < (sizeof(knownBLEAddresses) / sizeof(knownBLEAddresses[0])); i++)
      {
        //Uncomment to Enable Debug Information
        //Serial.println("*************Start**************");
        //Serial.println(sizeof(knownBLEAddresses));
        //Serial.println(sizeof(knownBLEAddresses[0]));
        //Serial.println(sizeof(knownBLEAddresses)/sizeof(knownBLEAddresses[0]));
        //Serial.println(advertisedDevice.getAddress().toString().c_str());
        //Serial.println(knownBLEAddresses[i].c_str());
        //Serial.println("*************End**************");
        if (strcmp(advertisedDevice.getAddress().toString().c_str(), knownBLEAddresses[i].c_str()) == 0)
                        {
          device_found = true;
          // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
                          break;
                        }
        else
          device_found = false;
      }
      
    }
};


void beepFlash( void * parameter) {
  for(;;) {
    if (failedScan == false) {
      //analogWrite(beepPin, 20);
      // digitalWrite(LED_BUILTIN, HIGH); // Uncomment this line to enable the built-in LED to blink.
      //digitalWrite(flashPin, HIGH);
      delay(100);
      //analogWrite(beepPin, 0);
      //digitalWrite(LED_BUILTIN, LOW);
      //digitalWrite(flashPin, LOW);
      //delay(beepFlashDelay);


      // Do some math on RSSI to determine how much to add to radiation level
      // want really high RSSI to quickly increase radiation level
      radiationValue += (100 + rssi) * 0.5;
      Serial.print("RSSI: ");
      Serial.println(rssi);
      Serial.print("Radiation Value: ");
      Serial.println(radiationValue);
      Serial.print("Radiation Stage: ");
      Serial.println(radiationLevel);

      // when radiation value has reached next threshold increment radiationLevel
      
          if (radiationValue > radiationThresholds[radiationLevel]) {
              digitalWrite(radiationPins[radiationLevel], HIGH);
              radiationLevel++;
          } 

      // Flash most recent LED
      digitalWrite(radiationPins[radiationLevel], HIGH);
      delay(250);
      digitalWrite(radiationPins[radiationLevel], LOW);
      delay(250);


    } else {
//      Serial.println("no device found");
      delay(50);
    }
  }
}


void setup() {
    Serial.begin(115200); 
    Serial.println("Starting setup...");

    pinMode(15, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(RX2, OUTPUT);
  pinMode(TX2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);


    Serial.println("LED pins initialized");

    xTaskCreatePinnedToCore(
        beepFlash, "Task1", 2048, NULL, 0, &Task1, 1);
    Serial.println("BeepFlash task created");

    Serial.println("Initializing BLE...");
    BLEDevice::init("");  
    Serial.println("BLE Initialized");

    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    Serial.println("Setup complete!");
}

void loop() {
  //digitalWrite(LED_BUILTIN, LOW);
 // digitalWrite(powerLEDPin, HIGH);
  lowestRssi = -1000;
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  
  numberOfDevices = foundDevices->getCount();
//  Serial.println(numberOfDevices);
  
  for (int i = 0; i < numberOfDevices; i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    rssi = device.getRSSI();
    //Serial.print("RSSI: ");
    //Serial.println(rssi);
    if (rssi > lowestRssi) {
      lowestRssi = rssi;
    }
  }
//  Serial.println(lowestRssi);
  if (numberOfDevices < 1) {
    if (missed > 2) {
      failedScan = true;
      Serial.println("scan failed");
    } else {
      missed++;
    }
    Serial.print("missed: ");
    Serial.println(missed);
  } else {
    beepFlashDelay = (((-(lowestRssi))-40)*5);
    missed = 0;
    failedScan = false;
  }
//  Serial.println(numberOfDevices);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}
