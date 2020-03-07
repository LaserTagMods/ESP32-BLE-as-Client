#include "BLEDevice.h"
//#include "BLEScan.h"

#define BLE_SERVER_SERVICE_NAME "jayeburden"

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
BLEClient*  pClient;

char notifyData[100];
int notifyDataIndex = 0;
String tokenStrings[100];
char *ptr = NULL;
int paired=0;

int settingsallowed = 0;
int SetSlotA;
int SetSlotB;
int SetSlotC;
int SetTeam;
int SetTime;
int SetODMode;
int SetWSMode;
int SetGNDR;
int SetLives;
int SetRSPNMode;
int SetKillCnt;
int SetObjct;
int SetFF;
int SetAmmo;
int AmmoGndr=0;

int PlayerKillCount[64] = {0}; // so its players 0-63 as the player id.
int TeamKillCount[6] = {0}; // teams 0-6, Red-0, blue-1, yellow-2, green-3, purple-4, cyan-5
int MaxKills = 32767; // setting limit on kill counts
int Objectives = 32000; // objective goals
int PlayerLives = 32767; // setting max player lives
int MaxTeamLives = 32767; // setting maximum team lives
long GameTimer = 2000000000; // setting maximum game time
int DelayStart = 30000; // set delay count down to 30 seconds for start
bool OutofAmmoA = false;
bool OutofAmmoB = false;

int lastTaggedPlayer = -1;
int lastTaggedTeam = -1;

bool RESPAWN = false;
bool GAMEOVER = false;

long startScan = 0;

bool WEAP = false;

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

  memcpy(notifyData + notifyDataIndex, pData, length);
  notifyDataIndex = notifyDataIndex + length;
  if (notifyData[notifyDataIndex - 1] == '*') { // complete receviing
    notifyData[notifyDataIndex] = '\0';
    notifyDataIndex = 0; // reset index
    //Lets tokenize by ","
    String receData = notifyData;
    byte index = 0;
    ptr = strtok((char*)receData.c_str(), ",");  // takes a list of delimiters
    while (ptr != NULL)
    {
      tokenStrings[index] = ptr;
      index++;
      ptr = strtok(NULL, ",");  // takes a list of delimiters
    }
    Serial.println("We have found " + String(index ) + " tokens");
    if (tokenStrings[0] == "$#CONNECT") {
      paired=1;
    }
    
    if (tokenStrings[0] == "$BUT") {
      if (tokenStrings[1] == "0") {
        if (tokenStrings[2] == "1") {
          Serial.println("Trigger pulled");
        }
        if (tokenStrings[2] == "0") {
          Serial.println("Trigger Released");
        }
      }
      if (tokenStrings[1] == "1") {
        if (tokenStrings[2] == "1") {
          Serial.println("Alt fire pulled");
        }
        if (tokenStrings[2] == "0") {
          Serial.println("Alt fire Released");
        }
      }
    }
    if (tokenStrings[0] == "$HIR") {
      /*space 1 is the direction the tag came from or what sensor was hit, in above example sensor 4
        space 2 is for he type of tag recieved/shot. almost all are 0 and im thinking other types are medic etc.
        space 3 is the player id, in this case it was player 0 hitting player 1.
        space 4 is the team id, this was team 0 or red team
        space 5 is the damage dealt to the player, this took 13 hit points off the player 1
        space 6 is "is critical" if critical a damage multiplier would apply, rare.
        space 7 is "power", not sure what that does.*/
      //been tagged
      lastTaggedPlayer = tokenStrings[3].toInt();
      lastTaggedTeam = tokenStrings[4].toInt();
      Serial.println("Just tagged by: " + String(lastTaggedPlayer) + " on team: " + String(lastTaggedTeam));
      if(settingsallowed==1) {
        roundonesettings();
      }
      if(settingsallowed==2) {
        roundtwosettings();
      }

    }
    if (tokenStrings[0] == "$ALCD") {
      /* ammunition status update occured 
       *  space 2 is rounds remaining in clip
       *  space 3 is accuracy of weapon
       *  space 4 is slot of weapon
       *  space 5 is remaining ammo outside clip
       *  space 6 is the overheating feature if aplicable
       *  
       *  more can be done with this, like using the ammo info to post to lcd
       */
      if ((tokenStrings[2] == "0") && (tokenStrings[4] == "0")) {
        if ((tokenStrings[3] == "0")) {
          OutofAmmoA=true;
        }
        else {
          OutofAmmoB=true;
        }
      }    
    }
    
    if (tokenStrings[0] == "$HP") {
      /*health status update occured
       * can be used for updates on health as well as death occurance
       */
      if ((tokenStrings[1] == "0") && (tokenStrings[2] == "0") && (tokenStrings[3] == "0")) {
        PlayerKillCount[lastTaggedPlayer]++;
        TeamKillCount[lastTaggedTeam]++;
        PlayerLives--;
        Serial.println("Lives Remaining = " + String(PlayerLives));
        Serial.println("Killed by: " + String(lastTaggedPlayer) + " on team: " + String(lastTaggedTeam));
        Serial.println("Team: " + String(lastTaggedTeam) + "Score: " + String(TeamKillCount[lastTaggedTeam]));
        Serial.println("Player: " + String(lastTaggedPlayer) + " Score: " + String(PlayerKillCount[lastTaggedPlayer]));
        if (PlayerLives > 0) {
          RESPAWN = true;
          Serial.println("respawn enabled");
          GAMEOVER = false;
        }
        else {
          GAMEOVER = true;
        }
      }
    }
  } else {
    //hold data and keep receving
  }
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      doConnect = true;
      doScan = true;
      Serial.println("onDisconnect");

    }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());


  Serial.println(" - Created client");



  // Connect to the remove BLE Server.
  if (pClient->connect(myDevice)) { // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
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
  return false;
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
      //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      if (advertisedDevice.getName() == BLE_SERVER_SERVICE_NAME) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = false;

      } // Found our server
      else {
        doScan = true;
      }
    } // onResult
}; // MyAdvertisedDeviceCallbacks

void notifyconnection() {
  sendString("$PLAY,VA20,3,9,,,,,*");
  Serial.println("sending connection notification");
  paired=0;
}

void taggedoutmode() {
  /*
  sendString("$START,*");
  sendString("$GSET,1,0,1,0,1,0,50,1,*");
  sendString("$PSET,54,5,45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*");
  sendString("$SIR,0,0,,1,0,0,1,,*");
  sendString("$SIR,0,1,,36,0,0,1,,*");
  sendString("$SIR,0,3,,37,0,0,1,,*");
  sendString("$SIR,8,0,,38,0,0,1,,*");
  sendString("$SIR,9,3,,24,10,0,,,*");
  sendString("$SIR,10,0,X13,1,0,100,2,60,*");
  sendString("$SIR,6,0,H02,1,0,90,1,40,*");
  sendString("$SIR,13,1,H57,1,0,0,1,,*");
  sendString("$SIR,13,0,H50,1,0,0,1,,*");
  sendString("$SIR,13,3,H49,1,0,100,0,60,*");
  sendString("$BMAP,0,0,,,,,*");
  sendString("$BMAP,1,100,0,1,99,99,*");
  sendString("$BMAP,2,97,,,,,*");
  sendString("$BMAP,3,98,,,,,*");
  sendString("$BMAP,4,98,,,,,*");
  sendString("$BMAP,5,98,,,,,*");
  sendString("$BMAP,8,4,,,,,*");
  sendString("$SPAWN,,*");
  sendString("$WEAP,0,*"); // cleared out weapon 0
  sendString("$WEAP,1,*"); // cleared out weapon 1
  sendString("$WEAP,4,*"); // cleared out melee weapon
  sendString("$GLED,,,,5,,,*"); // changes headset to tagged out color
  */
  RESPAWN = false;
}

void getsettings() {
  sendString("$START,*");
  sendString("$GSET,1,0,1,0,1,0,50,1,*");
  sendString("$PSET,54,5,45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*");
  //sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  //sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  //sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  sendString("$VOL,0,0,*"); // sets max volume on gun 0-100 feet distance
  sendString("$SIR,0,0,,1,0,0,1,,*");
  sendString("$SIR,0,1,,36,0,0,1,,*");
  sendString("$SIR,0,3,,37,0,0,1,,*");
  sendString("$SIR,8,0,,38,0,0,1,,*");
  sendString("$SIR,9,3,,24,10,0,,,*");
  sendString("$SIR,10,0,X13,1,0,100,2,60,*");
  sendString("$SIR,6,0,H02,1,0,90,1,40,*");
  sendString("$SIR,13,1,H57,1,0,0,1,,*");
  sendString("$SIR,13,0,H50,1,0,0,1,,*");
  sendString("$SIR,13,3,H49,1,0,100,0,60,*");
  sendString("$BMAP,0,0,,,,,*");
  sendString("$BMAP,1,100,0,1,99,99,*");
  sendString("$BMAP,2,97,,,,,*");
  sendString("$BMAP,3,98,,,,,*");
  sendString("$BMAP,4,98,,,,,*");
  sendString("$BMAP,5,98,,,,,*");
  sendString("$BMAP,8,4,,,,,*");
  //sendString("$PLAYX,0,*");
  //sendString("$PLAY,VA81,4,6,,,,,*");
  sendString("$SPAWN,,*");
  //WEAP = true;

  sendString("$WEAP,0,*"); // cleared out weapon 0
  sendString("$WEAP,1,*"); // cleared out weapon 1
  sendString("$WEAP,4,*"); // cleared out melee weapon
  //sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  //sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  //sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  //sendString("$HLOOP,0,0,*"); // not sure what this does
  sendString("$GLED,,,,5,,,*"); // changes headset to tagged out color
  //sendString("$WEAP,0,*"); // cleared out weapon 0
  //sendString("$WEAP,1,*"); // cleared out weapon 1
  //sendString("$WEAP,4,*"); // cleared out melee weapon
  //sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  //sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  //sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  //sendString("$PLAYX,0,*");
  //sendString("$PLAY,VA81,4,6,,,,,*");
  //sendString("$SPAWN,,*"); // respawns player back in game
  //Serial.println("Player Respawned");
  //RESPAWN = false;
  settingsallowed=1;
}

void roundonesettings() {
  SetSlotA=lastTaggedPlayer;
  SetTeam=lastTaggedTeam;
  SetTime=tokenStrings[2].toInt();
  SetODMode=tokenStrings[5].toInt();
  SetWSMode=tokenStrings[6].toInt();
  AmmoGndr=tokenStrings[7].toInt();
  if(AmmoGndr == 0) {SetGNDR=0; SetAmmo=0;}
  if(AmmoGndr == 1) {SetGNDR=1; SetAmmo=0;}
  if(AmmoGndr == 2) {SetGNDR=0; SetAmmo=1;}
  if(AmmoGndr == 3) {SetGNDR=1; SetAmmo=1;}
  settingsallowed=2;
  Serial.println("Round 1 settings loaded!");
}

void roundtwosettings() {
        SetSlotB=lastTaggedPlayer;
        SetKillCnt=lastTaggedTeam;
        SetLives=tokenStrings[2].toInt();
        SetObjct=tokenStrings[5].toInt();
        SetRSPNMode=tokenStrings[6].toInt();
        SetFF=tokenStrings[7].toInt();
        settingsallowed=3;
        Serial.println("Round 2 settings loaded!");

}

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
    pRemoteRXCharacteristic->writeValue((uint8_t*)value.c_str(), value.length(), true);
  }
}

void gametimesettings() {
  /* ideally we want setting options for minutes: 1,3,5,10,20,30,60, unlimited
   *  we will be using the bullet type ir or blynk for setting the time limits
   */
   if (SetTime = 0) {GameTimer=60000;} // timer set to one minute
   if (SetTime = 1) {GameTimer=180000;} // timer set to 3 minutes
   if (SetTime = 2) {GameTimer=300000;} // timer set to 5 minutes
   if (SetTime = 3) {GameTimer=600000;} // timer set to 10 minutes
   if (SetTime = 4) {GameTimer=1200000;} // timer set to 20 minutes
   if (SetTime = 5) {GameTimer=1800000;} // timer set to 30 minutes
   if (SetTime = 6) {GameTimer=3600000;} // timer set to 60 minutes
   if (SetTime > 6) {GameTimer=2000000000;} // set to unlimited minutes
}
void gamelivessettings() {
  /* ideally we want setting options for minutes: 1,3,5,10,20,30,60, unlimited
   *  we will be using the bullet type ir or blynk for setting the time limits
   */
   if (SetLives = 0) {PlayerLives=1;} // lives set to one 
   if (SetLives = 1) {PlayerLives=3;} // lives set to 3 
   if (SetLives = 2) {PlayerLives=5;} // lives set to 5 
   if (SetLives = 3) {PlayerLives=10;} // lives set to 10 
   if (SetLives = 4) {PlayerLives=20;} // lives set to 20 
   if (SetLives = 5) {PlayerLives=30;} // lives set to 30 
   if (SetLives = 6) {PlayerLives=60;} // lives set to 60 
   if (SetLives > 6) {PlayerLives=32000;} // set to unlimited 
}
void killcountsettings() {
  /* ideally we want setting options for max kills: 10,20,50, unlimited
   *  we will be using the team type ir or blynk for setting the time limits
   */
   if (SetKillCnt = 0) {MaxKills=10;} // max kills set to 10 
   if (SetKillCnt = 1) {MaxKills=20;} // max kills set to 20 
   if (SetKillCnt = 2) {MaxKills=50;} // max kills set to 50 
   if (SetKillCnt = 3) {MaxKills=32000;} // max kills set to unlimited
}
void objectivesettings() {
  /* ideally we want setting options for objectives: 10,20,50, unlimited
   *  we will be using the team type ir or blynk for setting the time limits
   */
   if (SetObjct = 1) {Objectives=1;} // objectives set to 1 
   if (SetObjct = 2) {Objectives=3;} // objectives set to 3 
   if (SetObjct = 3) {Objectives=5;} // objectives set to 5 
   if (SetObjct = 4) {Objectives=10;} // objectives set to 10
   if (SetObjct > 4) {Objectives=32000;} // objectives set to unlimited
}

void playersettings() {
  if(SetGNDR == 0) {sendString("$PSET,0,"+String(SetTeam)+",45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*");}
  else {sendString("$PSET,0,"+String(SetTeam)+",45,70,70,50,,H44,JAD,VB3,VBI,VBC,VBG,VBE,VB7,H06,H55,H13,H21,H02,U15,W71,A10,*");}
}

void weaponsettingsA() {
  if(SetSlotA == 1) {sendString("$WEAP,0,*");} // cleared out weapon 0
  if(SetSlotA == 2) {sendString("$WEAP,0,,100,0,3,18,0,,,,,,,,360,850,14,56,1400,0,7,100,100,,0,,,S07,D20,D19,,D04,D03,D21,D18,,,,,14,28,75,,*");}
  if(SetSlotA == 3) {sendString("$WEAP,0,,100,0,0,9,0,,,,,,,,100,850,32,384,1400,0,0,100,100,,0,,,R01,,,,D04,D03,D02,D18,,,,,32,192,75,,*");}
  if(SetSlotA == 4) {sendString("$WEAP,0,,100,0,3,13,0,,,,,,,,225,850,18,180,2000,0,7,100,100,,0,,,R12,,,,D04,D03,D02,D18,,,,,18,90,75,,*");}
  if(SetSlotA == 5) {sendString("$WEAP,0,,100,0,3,9,0,,,,,,,,75,850,36,216,1700,0,9,100,100,275,0,,,R18,,,,D04,D03,D02,D18,,,,,36,108,75,,*");}
  if(SetSlotA == 6) {sendString("$WEAP,0,,100,8,0,100,0,,,,,,,,1250,850,100,200,2500,0,14,100,100,,14,,,E03,C15,C17,,D30,D29,D37,A73,C19,C04,20,150,100,100,75,,*");}
  if(SetSlotA == 7) {sendString("$WEAP,0,,100,9,3,115,0,,,,,,,,360,850,1,6,1400,0,0,100,100,,0,,,J15,,,,D14,D13,D12,D18,,,,,1,3,75,,*");}
  if(SetSlotA == 8) {sendString("$WEAP,0,,100,0,0,9,0,,,,,,,,90,850,300,600,2400,0,0,100,100,,6,,,E12,,,,D17,D16,D15,A73,D122,,,,300,300,75,,*");}
  if(SetSlotA == 9) {sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,,*");}
  if(SetSlotA == 10) {sendString("$WEAP,0,,100,0,0,115,0,,,,,,,,1000,850,2,12,2000,0,7,100,100,,0,,,E07,D32,D31,,D17,D16,D15,A73,,,,,2,6,75,,*");}
  if(SetSlotA == 11) {sendString("$WEAP,0,,100,0,0,115,0,,,,,,,,1500,850,4,8,2000,0,3,100,100,,0,,,C06,C11,,,D17,D16,D15,A73,,,,,4,4,75,,*");}
  if(SetSlotA == 12) {sendString("$WEAP,0,2,100,0,0,80,0,,,,,,80,80,225,850,10,80,2000,0,7,100,100,,30,,,E17,,,,D35,D34,D36,A73,D122,,,,10,40,75,40,*");}
  if(SetSlotA == 13) {sendString("$WEAP,0,0,100,6,0,115,0,,,,,,,,1200,850,1,6,2400,0,2,100,100,,0,,,C03,C08,,,D36,D35,D34,A73,,,,,1,3,75,,*");}
  if(SetSlotA == 14) {sendString("$WEAP,0,2,100,10,0,115,0,,,,,,115,80,1000,850,2,8,1200,0,7,100,100,,0,,,C03,,,,D14,D13,D12,D18,,,,,2,4,75,30,*");}
  if(SetSlotA == 15) {sendString("$WEAP,0,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");}
  if(SetSlotA == 16) {sendString("$WEAP,0,,100,0,0,8,0,,,,,,,,90,850,72,288,2500,0,0,100,100,,5,,,G03,,,,D26,D25,D24,D18,D11,,,,72,144,75,,*");}
  if(SetSlotA == 17) {sendString("$WEAP,0,,100,0,1,80,0,,,,,,,,300,850,4,24,1700,0,7,100,100,,0,,,S16,D20,D19,,D04,D03,D21,D18,,,,,4,12,75,,*");}
  if(SetSlotA == 18) {sendString("$WEAP,0,,100,0,0,15,0,,,,,,,,120,850,18,72,1700,0,0,100,100,,0,,,E11,,,,D17,D16,D15,A73,,,,,18,36,75,,*");}
  if(SetSlotA == 19) {sendString("$WEAP,0,,100,0,0,8,0,,,,,,,,75,850,48,288,2000,0,0,100,100,,0,2,50,Q06,,,,D26,D25,D24,D18,,,,,48,144,75,,*");}
}

void weaponsettingsB() {
  if(SetSlotB == 1) {sendString("$WEAP,1,*");} // cleared out weapon 1
  if(SetSlotB == 2) {sendString("$WEAP,0,,100,0,3,18,0,,,,,,,,360,850,14,56,1400,0,7,100,100,,0,,,S07,D20,D19,,D04,D03,D21,D18,,,,,14,28,75,,*");}
  if(SetSlotB == 3) {sendString("$WEAP,0,,100,0,0,9,0,,,,,,,,100,850,32,384,1400,0,0,100,100,,0,,,R01,,,,D04,D03,D02,D18,,,,,32,192,75,,*");}
  if(SetSlotB == 4) {sendString("$WEAP,0,,100,0,3,13,0,,,,,,,,225,850,18,180,2000,0,7,100,100,,0,,,R12,,,,D04,D03,D02,D18,,,,,18,90,75,,*");}
  if(SetSlotB == 5) {sendString("$WEAP,0,,100,0,3,9,0,,,,,,,,75,850,36,216,1700,0,9,100,100,275,0,,,R18,,,,D04,D03,D02,D18,,,,,36,108,75,,*");}
  if(SetSlotB == 6) {sendString("$WEAP,0,,100,8,0,100,0,,,,,,,,1250,850,100,200,2500,0,14,100,100,,14,,,E03,C15,C17,,D30,D29,D37,A73,C19,C04,20,150,100,100,75,,*");}
  if(SetSlotB == 7) {sendString("$WEAP,0,,100,9,3,115,0,,,,,,,,360,850,1,6,1400,0,0,100,100,,0,,,J15,,,,D14,D13,D12,D18,,,,,1,3,75,,*");}
  if(SetSlotB == 8) {sendString("$WEAP,0,,100,0,0,9,0,,,,,,,,90,850,300,600,2400,0,0,100,100,,6,,,E12,,,,D17,D16,D15,A73,D122,,,,300,300,75,,*");}
  if(SetSlotB == 9) {sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,,*");}
  if(SetSlotB == 10) {sendString("$WEAP,0,,100,0,0,115,0,,,,,,,,1000,850,2,12,2000,0,7,100,100,,0,,,E07,D32,D31,,D17,D16,D15,A73,,,,,2,6,75,,*");}
  if(SetSlotB == 11) {sendString("$WEAP,0,,100,0,0,115,0,,,,,,,,1500,850,4,8,2000,0,3,100,100,,0,,,C06,C11,,,D17,D16,D15,A73,,,,,4,4,75,,*");}
  if(SetSlotB == 12) {sendString("$WEAP,0,2,100,0,0,80,0,,,,,,80,80,225,850,10,80,2000,0,7,100,100,,30,,,E17,,,,D35,D34,D36,A73,D122,,,,10,40,75,40,*");}
  if(SetSlotB == 13) {sendString("$WEAP,0,0,100,6,0,115,0,,,,,,,,1200,850,1,6,2400,0,2,100,100,,0,,,C03,C08,,,D36,D35,D34,A73,,,,,1,3,75,,*");}
  if(SetSlotB == 14) {sendString("$WEAP,0,2,100,10,0,115,0,,,,,,115,80,1000,850,2,8,1200,0,7,100,100,,0,,,C03,,,,D14,D13,D12,D18,,,,,2,4,75,30,*");}
  if(SetSlotB == 15) {sendString("$WEAP,0,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");}
  if(SetSlotB == 16) {sendString("$WEAP,0,,100,0,0,8,0,,,,,,,,90,850,72,288,2500,0,0,100,100,,5,,,G03,,,,D26,D25,D24,D18,D11,,,,72,144,75,,*");}
  if(SetSlotB == 17) {sendString("$WEAP,0,,100,0,1,80,0,,,,,,,,300,850,4,24,1700,0,7,100,100,,0,,,S16,D20,D19,,D04,D03,D21,D18,,,,,4,12,75,,*");}
  if(SetSlotB == 18) {sendString("$WEAP,0,,100,0,0,15,0,,,,,,,,120,850,18,72,1700,0,0,100,100,,0,,,E11,,,,D17,D16,D15,A73,,,,,18,36,75,,*");}
  if(SetSlotB == 19) {sendString("$WEAP,0,,100,0,0,8,0,,,,,,,,75,850,48,288,2000,0,0,100,100,,0,2,50,Q06,,,,D26,D25,D24,D18,,,,,48,144,75,,*");} 
}

void SetFFOutdoor() {
  if(SetODMode == 0 && SetFF == 1) {sendString("$GSET,1,0,1,0,1,0,50,1,*");}
  if(SetODMode == 1 && SetFF == 1) {sendString("$GSET,1,1,1,0,1,0,50,1,*");}
  if(SetODMode == 1 && SetFF == 0) {sendString("$GSET,0,1,1,0,1,0,50,1,*");}
  if(SetODMode == 0 && SetFF == 0) {sendString("$GSET,0,0,1,0,1,0,50,1,*");}
}

void respawnplayer() {
  Serial.println("Respawning Player");

  sendString("$WEAP,0,*"); // cleared out weapon 0
  sendString("$WEAP,1,*"); // cleared out weapon 1
  sendString("$WEAP,4,*"); // cleared out melee weapon
  weaponsettingsA();
  weaponsettingsB();
  //sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  //sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  sendString("$HLOOP,0,0,*"); // not sure what this does
  sendString("$GLED,,,,5,,,*"); // changes headset to tagged out color
  delay(3000);
  sendString("$WEAP,0,*"); // cleared out weapon 0
  sendString("$WEAP,1,*"); // cleared out weapon 1
  sendString("$WEAP,4,*"); // cleared out melee weapon
  weaponsettingsA();
  weaponsettingsB();
  //sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  //sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  //sendString("$PLAYX,0,*");
  //sendString("$PLAY,VA81,4,6,,,,,*");
  sendString("$SPAWN,,*"); // respawns player back in game
  Serial.println("Player Respawned");
  RESPAWN = false;
}

void samplesendsettings() {
  sendString("$START,*");
  sendString("$GSET,1,0,1,0,1,0,50,1,*");
  sendString("$PSET,54,5,45,70,70,50,,H44,JAD,V33,V3I,V3C,V3G,V3E,V37,H06,H55,H13,H21,H02,U15,W71,A10,*");
  sendString("$WEAP,0,,100,0,1,9,0,,,,,,,,100,850,36,144,1700,0,9,100,100,250,0,,,R23,D20,D19,,D23,D22,D21,D18,,,,,36,72,75,*");
  sendString("$WEAP,1,2,100,0,0,45,0,,,,,,70,80,900,850,6,24,400,2,7,100,100,,0,,,T01,,,,D01,D28,D27,D18,,,,,6,12,75,30,*");
  sendString("$WEAP,4,1,90,13,1,90,0,,,,,,,,1000,100,1,0,0,10,13,100,100,,0,0,,M92,,,,,,,,,,,,1,0,20,*");
  sendString("$SIR,0,0,,1,0,0,1,,*");
  sendString("$SIR,0,1,,36,0,0,1,,*");
  sendString("$SIR,0,3,,37,0,0,1,,*");
  sendString("$SIR,8,0,,38,0,0,1,,*");
  sendString("$SIR,9,3,,24,10,0,,,*");
  sendString("$SIR,10,0,X13,1,0,100,2,60,*");
  sendString("$SIR,6,0,H02,1,0,90,1,40,*");
  sendString("$SIR,13,1,H57,1,0,0,1,,*");
  sendString("$SIR,13,0,H50,1,0,0,1,,*");
  sendString("$SIR,13,3,H49,1,0,100,0,60,*");
  sendString("$BMAP,0,0,,,,,*");
  sendString("$BMAP,1,100,0,1,99,99,*");
  sendString("$BMAP,2,97,,,,,*");
  sendString("$BMAP,3,98,,,,,*");
  sendString("$BMAP,4,98,,,,,*");
  sendString("$BMAP,5,98,,,,,*");
  sendString("$BMAP,8,4,,,,,*");
  //sendString("$PLAYX,0,*");
  //sendString("$PLAY,VA81,4,6,,,,,*");
  sendString("$SPAWN,,*");
  WEAP = true;
}
      

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  delay(5000);
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10, true);
  BLEDevice::setPower(ESP_PWR_LVL_N14);

  pClient  = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

} // End of setup.



// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if(paired == 1) {notifyconnection();}
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
      doConnect = false; // to try and make the connection again.
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    // Set the characteristic's value to be the array of bytes that is actually a string.
    //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    //pRemoteCharacteristic->writeValue((uint8_t*)newValue.c_str(), newValue.length(),true);
    if (settingsallowed == 0) {
      getsettings();
    }
    if (RESPAWN) {
      if (SetRSPNMode == 0) {
      respawnplayer();
      }
      else {
        taggedoutmode();
      }
    }
    if (GAMEOVER) {
      gameover();
    }
    if (OutofAmmoA) {
      if (SetAmmo == 1) {
        weaponsettingsA();
      }
    }
    if (OutofAmmoB) {
      if (SetAmmo == 1) {
        weaponsettingsB();
      }
    }
  } else if (doScan) {
    if (millis() - startScan > 11000) {
      Serial.println("Scanning again");
      BLEDevice::init("");
      BLEDevice::getScan()->start(10, true); // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
      startScan = millis();
    }
  }

  delay(1000); // Delay a second between loops.
} // End of loop

void gameover() {
  sendString("STOP,*"); // stops everything going on
  sendString("CLEAR,*"); // clears out anything stored for game settings
  sendString("$PLAY,VS6,4,6,,,,,*"); // says game over
  GAMEOVER = false;
}
