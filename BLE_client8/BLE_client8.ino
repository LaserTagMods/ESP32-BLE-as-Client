/**
   A BLE client example that is rich in capabilities.
   There is a lot new capabilities implemented.
   author unknown
   updated by chegewara
*/
/*
   pRemoteCharacteristic->writeValue(value, length, true); -> opcode 0x12, request command
  pRemoteCharacteristic->writeValue(value, length); -> opcode 0x52 no sepsone command
  HANDLE <- data: host sends data to device using the ATT write-no-response command (opcode 0x52)
  HANDLE <-- data: host sends data to device using the ATT write-request command (opcode 0x12)
*/
/*
  data strings that need to be sent to start a callsign game:
  $CLEAR,*\r\n
  $START,*\r\n
  $GSET,1,0,1,0,1,0,50,1,*\r\n
  $PSET,0,0,45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*\r\n
  $WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*\r\n
  $WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*\r\n
  $WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*\r\n
  $SIR,0,0,,1,0,0,1,,*\r\n
  $SIR,0,1,,36,0,0,1,,*\r\n
  $SIR,0,3,,37,0,0,1,,*\r\n
  $SIR,8,0,,38,0,0,1,,*\r\n
  $SIR,9,3,,24,10,0,,,*\r\n
  $SIR,10,0,X13,1,0,100,2,60,*\r\n
  $SIR,6,0,H02,1,0,90,1,40,*\r\n
  $SIR,13,1,H57,1,0,0,1,,*\r\n
  $SIR,13,0,H50,1,0,0,1,,*\r\n
  $SIR,13,3,H49,1,0,100,0,60,*\r\n
  $BMAP,0,0,,,,,*\r\n
  $BMAP,1,100,0,1,99,99,*\r\n
  $BMAP,2,97,,,,,*\r\n
  $BMAP,3,98,,,,,*\r\n
  $BMAP,4,98,,,,,*\r\n
  $BMAP,5,98,,,,,*\r\n
  $BMAP,8,4,,,,,*\r\n
  $PLAYX,0,*\r\n
  $PLAY,VA81,4,6,,,,,*\r\n
  $SPAWN,,*\r\n
*/
#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
// The characteristic of the remote service we are interested in.
static BLEUUID    charRXUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID    charTXUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteRXCharacteristic;
static BLERemoteCharacteristic* pRemoteTXCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      Serial.println("onDisconnect");
    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteRXCharacteristic = pRemoteService->getCharacteristic(charRXUUID);
  if (pRemoteRXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charRXUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our RX characteristic");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteTXCharacteristic = pRemoteService->getCharacteristic(charTXUUID);
  if (pRemoteTXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charTXUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our TX characteristic");

  // Read the value of the characteristic.
  if (pRemoteRXCharacteristic->canRead()) {
    std::string value = pRemoteRXCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteTXCharacteristic->canRead()) {
    std::string value = pRemoteTXCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteTXCharacteristic->canNotify())
    pRemoteTXCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());

      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

bool WEAP = false;

// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    //String newValue = "Time since boot: " + String(millis()/1000);
    //String newValue = String("$PLAY,VS6,4,6,,,,,*");
    //String value = String("$WEAP,0,,100,0,3,18,0,,,,,,,,360,850,14,56,1400,0,7,100,100,,0,,,S07,D20,D19,,D04,D03,D21,D18,,,,,14,28,75,,*");
    //Serial.println("Setting new characteristic value to ");
    //Serial.println(value);

    // Set the characteristic's value to be the array of bytes that is actually a string.
    //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    //pRemoteCharacteristic->writeValue((uint8_t*)newValue.c_str(), newValue.length(),true);
     // sending strings of data - partial to test out trying to start a callsign match and listen in to responses from brx
    if (WEAP == false) {
      sendString("$START,*");
      sendString("$GSET,1,0,1,0,1,0,50,1,*");
      sendString("$PSET,0,0,45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*");
      sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");

    }
  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(1000); // Delay a second between loops.
} // End of loop

void sendString(String value) {

  const char * c_string = value.c_str();
  uint8_t buf[21] = {0};
  int sentSize = 0;
  Serial.println("sending ");
  if (value.length() > 20) {
    for (int i = 0; i < value.length() / 20; i++) {

      memcpy(buf, c_string + i * 20, 20);
      Serial.print((char*)buf);
      pRemoteRXCharacteristic->writeValue(buf, 20, true);
      sentSize += 20;
    }
    int remaining = value.length() - sentSize;
    memcpy(buf, c_string + sentSize, remaining);


    pRemoteRXCharacteristic->writeValue(buf, remaining, true);

    for (int i = 0; i < remaining; i++)
      Serial.print((char)buf[i]);
    Serial.println();
  }
  else {
    pRemoteRXCharacteristic->writeValue((uint8_t*)value.c_str(), value.length(),true);
  }
  WEAP = true;

}
