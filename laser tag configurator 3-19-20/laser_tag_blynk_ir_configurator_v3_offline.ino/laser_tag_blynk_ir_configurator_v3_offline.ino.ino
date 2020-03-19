/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest
 *************************************************************
  This example runs directly on ESP8266 chip.
  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino
  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!
  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 ************************************************************* 
  This Sketch is set up to create a specialy modified tag that
  will be recognized by the laser tag hardware and if paired 
  to an esp32 with the ir configuration code uploaded will be
  able to program one or multiple brx rifles at the same time
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "A1Nn_60FJGQvidn87-AzJcipwAjOtIJO";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ASUS_00_2G";
char pass[] = "12345678";

// BRX Configurations:
int PlayerID=0; // used to identify player
int PID[6]; // used for recording player bits for ID decifering
int parity = 0; // 0 = even; 1 = odd
int parityB = 0; // 0 = even; 1 = odd
int ActionState = 0; // variable used for storing a State
int IRledPin1 = D2; // LED connected to digital pin 2 output to send IR
int RXLED = 17; // The RX LED has a defined Arduino pin
int TXLED = 30; // The TX LED has a defined Arduino pin
int B[4];
int P[6];
int T[2];
int D[8];
int C[1];
int U[2];
int Z[2];
int BB[4];
int PP[6];
int TT[2];
int DD[8];
int CC[1];
int UU[2];
int ZZ[2];
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
// This procedure sends a 38KHz pulse to the IRledPin 
// for a certain # of microseconds. We'll use this whenever we need to send codes
void pulseIR1(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait 
  cli();  // this turns off any background interrupts
  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
   digitalWrite(IRledPin1, HIGH);  // this takes about 3 microseconds to happen
   delayMicroseconds(10);         // hang out for 10 microseconds, you can also change this to 9 if its not working
   digitalWrite(IRledPin1, LOW);   // this also takes about 3 microseconds
   delayMicroseconds(10);         // hang out for 10 microseconds, you can also change this to 9 if its not working
   // so 26 microseconds altogether
   microsecs -= 26;
  }
  sei();  // this turns them back on
}
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
// this is the function that actually sends the IR that has been preset by Blynk
void SendIR() {
  Serial.println("Sending IR1");
  // sending IR protocol on first pin
  pulseIR1(2150);
  delayMicroseconds(550);
  pulseIR1(B[0]); // B1
  delayMicroseconds(550);
  pulseIR1(B[1]); // B2
  delayMicroseconds(550);
  pulseIR1(B[2]); // B3
  delayMicroseconds(550);
  pulseIR1(B[3]); // B4
  delayMicroseconds(550);
  pulseIR1(P[0]); // P1
  delayMicroseconds(550);
  pulseIR1(P[1]); // P2
  delayMicroseconds(550);
  pulseIR1(P[2]); // P3
  delayMicroseconds(550);
  pulseIR1(P[3]); // P4
  delayMicroseconds(550);
  pulseIR1(P[4]); // P5
  delayMicroseconds(550);
  pulseIR1(P[5]); // P6
  delayMicroseconds(550);
  pulseIR1(T[0]); // T1
  delayMicroseconds(550);
  pulseIR1(T[1]);  // T2
  delayMicroseconds(550);
  pulseIR1(D[0]);  // D1
  delayMicroseconds(550);
  pulseIR1(D[1]);  // D2
  delayMicroseconds(550);
  pulseIR1(D[2]); // D3
  delayMicroseconds(550);
  pulseIR1(D[3]);  // D4
  delayMicroseconds(550);
  pulseIR1(D[4]);  // D5
  delayMicroseconds(550);
  pulseIR1(D[5]);  // D6
  delayMicroseconds(550);
  pulseIR1(D[6]);  // D7
  delayMicroseconds(550);
  pulseIR1(D[7]);  // D8
  delayMicroseconds(550);
  pulseIR1(C[0]);  // C1
  delayMicroseconds(550);
  pulseIR1(U[0]);  // U1
  delayMicroseconds(550);
  pulseIR1(U[1]);  // U2
  delayMicroseconds(550);
  pulseIR1(Z[0]); // Z1
  delayMicroseconds(550);
  pulseIR1(Z[1]); // Z2

  // this is a break between sending round one settings and round 2 settings
  delay(1100);

  Serial.println("Sending IR1");
  // sending IR protocol on first pin
  pulseIR1(2150);
  delayMicroseconds(550);
  pulseIR1(BB[0]); // B1
  delayMicroseconds(550);
  pulseIR1(BB[1]); // B2
  delayMicroseconds(550);
  pulseIR1(BB[2]); // B3
  delayMicroseconds(550);
  pulseIR1(BB[3]); // B4
  delayMicroseconds(550);
  pulseIR1(PP[0]); // P1
  delayMicroseconds(550);
  pulseIR1(PP[1]); // P2
  delayMicroseconds(550);
  pulseIR1(PP[2]); // P3
  delayMicroseconds(550);
  pulseIR1(PP[3]); // P4
  delayMicroseconds(550);
  pulseIR1(PP[4]); // P5
  delayMicroseconds(550);
  pulseIR1(PP[5]); // P6
  delayMicroseconds(550);
  pulseIR1(TT[0]); // T1
  delayMicroseconds(550);
  pulseIR1(TT[1]);  // T2
  delayMicroseconds(550);
  pulseIR1(DD[0]);  // D1
  delayMicroseconds(550);
  pulseIR1(DD[1]);  // D2
  delayMicroseconds(550);
  pulseIR1(DD[2]); // D3
  delayMicroseconds(550);
  pulseIR1(DD[3]);  // D4
  delayMicroseconds(550);
  pulseIR1(DD[4]);  // D5
  delayMicroseconds(550);
  pulseIR1(DD[5]);  // D6
  delayMicroseconds(550);
  pulseIR1(DD[6]);  // D7
  delayMicroseconds(550);
  pulseIR1(DD[7]);  // D8
  delayMicroseconds(550);
  pulseIR1(CC[0]);  // C1
  delayMicroseconds(550);
  pulseIR1(UU[0]);  // U1
  delayMicroseconds(550);
  pulseIR1(UU[1]);  // U2
  delayMicroseconds(550);
  pulseIR1(ZZ[0]); // Z1
  delayMicroseconds(550);
  pulseIR1(ZZ[1]); // Z2
}
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
void PrintTag() {
  // prints each individual bit on serial monitor
  Serial.println("Tag!!");
  Serial.print("B0: "); Serial.println(B[0]);
  Serial.print("B1: "); Serial.println(B[1]);
  Serial.print("B2: "); Serial.println(B[2]);
  Serial.print("B3: "); Serial.println(B[3]);
  Serial.print("P0: "); Serial.println(P[0]);
  Serial.print("P1: "); Serial.println(P[1]);
  Serial.print("P2: "); Serial.println(P[2]);
  Serial.print("P3: "); Serial.println(P[3]);
  Serial.print("P4: "); Serial.println(P[4]);
  Serial.print("P5: "); Serial.println(P[5]);
  Serial.print("T0: "); Serial.println(T[0]);
  Serial.print("T1: "); Serial.println(T[1]);
  Serial.print("D0: "); Serial.println(D[0]);
  Serial.print("D1: "); Serial.println(D[1]);
  Serial.print("D2: "); Serial.println(D[2]);
  Serial.print("D3: "); Serial.println(D[3]);
  Serial.print("D4: "); Serial.println(D[4]);
  Serial.print("D5: "); Serial.println(D[5]);
  Serial.print("D6: "); Serial.println(D[6]);
  Serial.print("D7: "); Serial.println(D[7]);
  Serial.print("C0: "); Serial.println(C[0]);
  Serial.print("U0: "); Serial.println(U[0]);
  Serial.print("U1: "); Serial.println(U[1]);
  Serial.print("Z0: "); Serial.println(Z[0]);
  Serial.print("Z1: "); Serial.println(Z[1]);
  Serial.println();
  Serial.println("Tag!!");
  Serial.print("BB0: "); Serial.println(BB[0]);
  Serial.print("BB1: "); Serial.println(BB[1]);
  Serial.print("BB2: "); Serial.println(BB[2]);
  Serial.print("BB3: "); Serial.println(BB[3]);
  Serial.print("PP0: "); Serial.println(PP[0]);
  Serial.print("PP1: "); Serial.println(PP[1]);
  Serial.print("PP2: "); Serial.println(PP[2]);
  Serial.print("PP3: "); Serial.println(PP[3]);
  Serial.print("PP4: "); Serial.println(PP[4]);
  Serial.print("PP5: "); Serial.println(PP[5]);
  Serial.print("TY0: "); Serial.println(TT[0]);
  Serial.print("TY1: "); Serial.println(TT[1]);
  Serial.print("DD0: "); Serial.println(DD[0]);
  Serial.print("DD1: "); Serial.println(DD[1]);
  Serial.print("DD2: "); Serial.println(DD[2]);
  Serial.print("DD3: "); Serial.println(DD[3]);
  Serial.print("DD4: "); Serial.println(DD[4]);
  Serial.print("DD5: "); Serial.println(DD[5]);
  Serial.print("DD6: "); Serial.println(DD[6]);
  Serial.print("DD7: "); Serial.println(DD[7]);
  Serial.print("CC0: "); Serial.println(CC[0]);
  Serial.print("UU0: "); Serial.println(UU[0]);
  Serial.print("UU1: "); Serial.println(UU[1]);
  Serial.print("ZZ0: "); Serial.println(ZZ[0]);
  Serial.print("ZZ1: "); Serial.println(ZZ[1]);
  Serial.println();
}
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
void paritycheck() {
  // parity changes, parity starts as even as if zero 1's
  // are present in the protocol recieved and being sent
  parity = 0;
  if (B[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (B[1] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (B[2] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (B[3] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[1] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[2] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[3] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[4] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (P[5] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (T[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (T[1] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[1] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[2] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[3] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[4] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[5] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[6] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (D[7] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (C[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (U[0] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (U[1] > 750) {
    if (parity == 0) {parity = 1;}
    else {parity = 0;}
  }
  if (parity == 0) {
    Z[0]=1050;
    Z[1]=525;
  }
  else {
    Z[0]=525;
    Z[1]=1050;
  }
  Serial.println("Modified Parity Bits:");
  Serial.println(Z[0]);
  Serial.println(Z[1]);

  // parity changes, parity starts as even as if zero 1's
  // are present in the protocol recieved and being sent
  parityB = 0;
  if (BB[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (BB[1] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (BB[2] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (BB[3] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[1] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[2] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[3] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[4] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (PP[5] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (TT[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (TT[1] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[1] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[2] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[3] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[4] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[5] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[6] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (DD[7] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (CC[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (UU[0] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (UU[1] > 750) {
    if (parityB == 0) {parityB = 1;}
    else {parityB = 0;}
  }
  if (parityB == 0) {
    ZZ[0]=1050;
    ZZ[1]=525;
  }
  else {
    ZZ[0]=525;
    ZZ[1]=1050;
  }
  Serial.println("Modified parityB Bits:");
  Serial.println(ZZ[0]);
  Serial.println(ZZ[1]);
}
//******************************************************************************************************************************************************************************************
//****************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
//******************************************************************************************************************************************************************************************
// These are the blynk button and menu options that configure the ir protocol:

// WEAPON 01 DROP DOWN SELECTION
BLYNK_WRITE(V0) {
int b=param.asInt();
if (b==1) {P[0]=550; P[1]=550; P[2]=550; P[3]=550; P[4]=550; P[5]=550;}
if (b==2) {P[0]=550; P[1]=550; P[2]=550; P[3]=550; P[4]=550; P[5]=1100;}
if (b==3) {P[0]=550; P[1]=550; P[2]=550; P[3]=550; P[4]=1100; P[5]=550;}
if (b==4) {P[0]=550; P[1]=550; P[2]=550; P[3]=550; P[4]=1100; P[5]=1100;}
if (b==5) {P[0]=550; P[1]=550; P[2]=550; P[3]=1100; P[4]=550; P[5]=550;}
if (b==6) {P[0]=550; P[1]=550; P[2]=550; P[3]=1100; P[4]=550; P[5]=1100;}
if (b==7) {P[0]=550; P[1]=550; P[2]=550; P[3]=1100; P[4]=1100; P[5]=550;}
if (b==8) {P[0]=550; P[1]=550; P[2]=550; P[3]=1100; P[4]=1100; P[5]=1100;}
if (b==9) {P[0]=550; P[1]=550; P[2]=1100; P[3]=550; P[4]=550; P[5]=550;}
if (b==10) {P[0]=550; P[1]=550; P[2]=1100; P[3]=550; P[4]=550; P[5]=1100;}
if (b==11) {P[0]=550; P[1]=550; P[2]=1100; P[3]=550; P[4]=1100; P[5]=550;}
if (b==12) {P[0]=550; P[1]=550; P[2]=1100; P[3]=550; P[4]=1100; P[5]=1100;}
if (b==13) {P[0]=550; P[1]=550; P[2]=1100; P[3]=1100; P[4]=550; P[5]=550;}
if (b==14) {P[0]=550; P[1]=550; P[2]=1100; P[3]=1100; P[4]=550; P[5]=1100;}
if (b==15) {P[0]=550; P[1]=550; P[2]=1100; P[3]=1100; P[4]=1100; P[5]=550;}
if (b==16) {P[0]=550; P[1]=550; P[2]=1100; P[3]=1100; P[4]=1100; P[5]=1100;}
if (b==17) {P[0]=550; P[1]=1100; P[2]=550; P[3]=550; P[4]=550; P[5]=550;}
if (b==18) {P[0]=550; P[1]=1100; P[2]=550; P[3]=550; P[4]=550; P[5]=1100;}
if (b==19) {P[0]=550; P[1]=1100; P[2]=550; P[3]=550; P[4]=1100; P[5]=550;}
if (b==20) {P[0]=550; P[1]=1100; P[2]=550; P[3]=550; P[4]=1100; P[5]=1100;}
if (b==21) {P[0]=550; P[1]=1100; P[2]=550; P[3]=1100; P[4]=550; P[5]=550;}
if (b==22) {P[0]=550; P[1]=1100; P[2]=550; P[3]=1100; P[4]=550; P[5]=1100;}
if (b==23) {P[0]=550; P[1]=1100; P[2]=550; P[3]=1100; P[4]=1100; P[5]=550;}
if (b==24) {P[0]=550; P[1]=1100; P[2]=550; P[3]=1100; P[4]=1100; P[5]=1100;}
if (b==25) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=550; P[4]=550; P[5]=550;}
if (b==26) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=550; P[4]=550; P[5]=1100;}
if (b==27) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=550; P[4]=1100; P[5]=550;}
if (b==28) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=550; P[4]=1100; P[5]=1100;}
if (b==29) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=1100; P[4]=550; P[5]=550;}
if (b==30) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=1100; P[4]=550; P[5]=1100;}
if (b==31) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=1100; P[4]=1100; P[5]=550;}
if (b==32) {P[0]=550; P[1]=1100; P[2]=1100; P[3]=1100; P[4]=1100; P[5]=1100;}
paritycheck();
}
//************************************************************************
// WEAPON 02 DROP DOWN SELECTION
BLYNK_WRITE(V8) {
int b=param.asInt();
if (b==1) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=550; PP[4]=550; PP[5]=550;}
if (b==2) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=550; PP[4]=550; PP[5]=1100;}
if (b==3) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=550; PP[4]=1100; PP[5]=550;}
if (b==4) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=550; PP[4]=1100; PP[5]=1100;}
if (b==5) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=1100; PP[4]=550; PP[5]=550;}
if (b==6) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=1100; PP[4]=550; PP[5]=1100;}
if (b==7) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=1100; PP[4]=1100; PP[5]=550;}
if (b==8) {PP[0]=550; PP[1]=550; PP[2]=550; PP[3]=1100; PP[4]=1100; PP[5]=1100;}
if (b==9) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=550; PP[4]=550; PP[5]=550;}
if (b==10) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=550; PP[4]=550; PP[5]=1100;}
if (b==11) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=550; PP[4]=1100; PP[5]=550;}
if (b==12) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=550; PP[4]=1100; PP[5]=1100;}
if (b==13) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=1100; PP[4]=550; PP[5]=550;}
if (b==14) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=1100; PP[4]=550; PP[5]=1100;}
if (b==15) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=1100; PP[4]=1100; PP[5]=550;}
if (b==16) {PP[0]=550; PP[1]=550; PP[2]=1100; PP[3]=1100; PP[4]=1100; PP[5]=1100;}
if (b==17) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=550; PP[4]=550; PP[5]=550;}
if (b==18) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=550; PP[4]=550; PP[5]=1100;}
if (b==19) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=550; PP[4]=1100; PP[5]=550;}
if (b==20) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=550; PP[4]=1100; PP[5]=1100;}
if (b==21) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=1100; PP[4]=550; PP[5]=550;}
if (b==22) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=1100; PP[4]=550; PP[5]=1100;}
if (b==23) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=1100; PP[4]=1100; PP[5]=550;}
if (b==24) {PP[0]=550; PP[1]=1100; PP[2]=550; PP[3]=1100; PP[4]=1100; PP[5]=1100;}
if (b==25) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=550; PP[4]=550; PP[5]=550;}
if (b==26) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=550; PP[4]=550; PP[5]=1100;}
if (b==27) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=550; PP[4]=1100; PP[5]=550;}
if (b==28) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=550; PP[4]=1100; PP[5]=1100;}
if (b==29) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=1100; PP[4]=550; PP[5]=550;}
if (b==30) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=1100; PP[4]=550; PP[5]=1100;}
if (b==31) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=1100; PP[4]=1100; PP[5]=550;}
if (b==32) {PP[0]=550; PP[1]=1100; PP[2]=1100; PP[3]=1100; PP[4]=1100; PP[5]=1100;}
paritycheck();
}
//****************************************************************************
// SETS MAX LIVES
BLYNK_WRITE(V9) {
int b=param.asInt();
if (b==1) {BB[0]=550; BB[1]=550; BB[2]=550; BB[3]=550;}
if (b==2) {BB[0]=550; BB[1]=550; BB[2]=550; BB[3]=1100;}
if (b==3) {BB[0]=550; BB[1]=550; BB[2]=1100; BB[3]=550;}
if (b==4) {BB[0]=550; BB[1]=550; BB[2]=1100; BB[3]=1100;}
if (b==5) {BB[0]=550; BB[1]=1100; BB[2]=550; BB[3]=550;}
if (b==6) {BB[0]=550; BB[1]=1100; BB[2]=550; BB[3]=1100;}
if (b==7) {BB[0]=550; BB[1]=1100; BB[2]=1100; BB[3]=550;}
if (b==8) {BB[0]=550; BB[1]=1100; BB[2]=1100; BB[3]=1100;}
paritycheck();
}
//****************************************************************************
// SETS GAME TIME LIMIT
BLYNK_WRITE(V5) {
int b=param.asInt();
if (b==1) {B[0]=550; B[1]=550; B[2]=550; B[3]=550;}
if (b==2) {B[0]=550; B[1]=550; B[2]=550; B[3]=1100;}
if (b==3) {B[0]=550; B[1]=550; B[2]=1100; B[3]=550;}
if (b==4) {B[0]=550; B[1]=550; B[2]=1100; B[3]=1100;}
if (b==5) {B[0]=550; B[1]=1100; B[2]=550; B[3]=550;}
if (b==6) {B[0]=550; B[1]=1100; B[2]=550; B[3]=1100;}
if (b==7) {B[0]=550; B[1]=1100; B[2]=1100; B[3]=550;}
if (b==8) {B[0]=550; B[1]=1100; B[2]=1100; B[3]=1100;}
paritycheck();
}
//****************************************************************************
// SETS PLAYER TEAM
BLYNK_WRITE(V6) {
int b=param.asInt();
if (b==1) {T[0]=550; T[1]=550;} // RED
if (b==2) {T[0]=550; T[1]=1100;} // BLUE
if (b==3) {T[0]=1100; T[1]=550;} // YELLOW
if (b==4) {T[0]=1100; T[1]=1100;} // GREEN
paritycheck();
}
//****************************************************************************
// SETS MAX KILLS
BLYNK_WRITE(V11) {
int b=param.asInt();
if (b==1) {TT[0]=550; TT[1]=550;} // 10
if (b==2) {TT[0]=550; TT[1]=1100;} // 25
if (b==3) {TT[0]=1100; TT[1]=550;} // 50
if (b==4) {TT[0]=1100; TT[1]=1100;} // UNLIMITED
paritycheck();
}
//****************************************************************************
// SETS MAX OBJECTIVES
BLYNK_WRITE(V12) {
int b=param.asInt();
if (b==1) {DD[0]=550; DD[1]=550; DD[2]=550; DD[3]=550; DD[4]=550; DD[5]=550; DD[6]=550; DD[7]=1100;} // 1
if (b==2) {DD[0]=550; DD[1]=550; DD[2]=550; DD[3]=550; DD[4]=550; DD[5]=550; DD[6]=1100; DD[7]=550;} // 3
if (b==3) {DD[0]=550; DD[1]=550; DD[2]=550; DD[3]=550; DD[4]=550; DD[5]=550; DD[6]=1100; DD[7]=1100;} // 5
if (b==4) {DD[0]=550; DD[1]=550; DD[2]=550; DD[3]=550; DD[4]=550; DD[5]=1100; DD[6]=550; DD[7]=550;} // 10
if (b==5) {DD[0]=550; DD[1]=550; DD[2]=550; DD[3]=550; DD[4]=550; DD[5]=1100; DD[6]=550; DD[7]=1100;} // UNLIMITED
paritycheck();
}
//****************************************************************************
// WEAPON SELECTION ON/OFF
BLYNK_WRITE(V1)
{
  int i=param.asInt();
  if (i==1)
  {
  C[0]=1050;
  }
  else {
    C[0]=525;
  }
  paritycheck();
}
//****************************************************************************
// OUTDOOR MODE ON/OFF
BLYNK_WRITE(V2)
{
  int i=param.asInt();
  if (i==1)
  {
  D[0]=550; D[1]=550; D[2]=550; D[3]=550; D[4]=550; D[5]=550; D[6]=1100; D[7]=550;
  }
  else {
    D[0]=550; D[1]=550; D[2]=550; D[3]=550; D[4]=550; D[5]=550; D[6]=550; D[7]=1100;
  }
  paritycheck();
}
//****************************************************************************
// GENDER SELECTION MALE/FEMALE
BLYNK_WRITE(V4)
{
  int i=param.asInt();
  if (i==1)
  {
  U[1]=1050;
  }
  else {
    U[1]=525;
  }
  paritycheck();
}
//****************************************************************************
// UNLIMITED AMMO ON/OFF
BLYNK_WRITE(V3)
{
  int i=param.asInt();
  if (i==1)
  {
  U[0]=525;
  }
  else {
    U[0]=1050;
  }
  paritycheck();
}
//****************************************************************************
// AUTO RESPAWN ON/OFF
BLYNK_WRITE(V10)
{
  int i=param.asInt();
  if (i==1)
  {
  CC[0]=1050;
  }
  else {
    CC[0]=525;
  }
  paritycheck();
}
//****************************************************************************
// UNLIMITED AMMO ON/OFF
BLYNK_WRITE(V13)
{
  int i=param.asInt();
  if (i==1)
  {
  UU[0]=525;  
  UU[1]=1050;
  }
  else {
    UU[0]=525;
    UU[1]=525;
  }
  paritycheck();
}
//****************************************************************************
// SENDS THE IR DATA
BLYNK_WRITE(V7)
{
  int i=param.asInt();
  if (i==1)
  {
  PrintTag();
  SendIR();
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,252), 8080);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RXLED, OUTPUT); // Set RX LED as an output
  pinMode(TXLED, OUTPUT); // Set TX LED as an output
  // initialize the IR digital pin as an output:
  pinMode(IRledPin1, OUTPUT);
}

void loop()
{
  Blynk.run();
}
