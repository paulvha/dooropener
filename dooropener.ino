/*
 * ============================================================================
 * 
 * Open the door with a password and/or NFC card.
 * 
 * ===========================================================================
 * 
 * HARDWARE Connection on UNO
 *
 * LCD UNO
 * SCL  A5
 * SDA  A4
 * VCC  5v
 * GND  GND
 * 
 * 
 * MFC 522
 *            UNO              MFC522
 * SPI SS     10               SDA
 * SPI SCK    13               SCK
 * SPI MOSI   11               MOSI
 * SPI MISO   12               MISO
 * IRQ                         IRQ
 * GND        GND              GND
 * Reset      9                RST
 * 3V3        3v3              3v3
 *
 *
 * KEYPAD 4 x 3
 * ROW = {5, A0, 7, 8};      // Rows 0 to 4
 * COLUMNs= {2, 3, 4};       // Columns 0 to 3
 * 
 * KEYPAD 4 x 4
 * ROWs {5, A0, 7, 8};       // Rows 0 to 4
 * COLUMNs = {2, 3, 4, A1};  // Columns 0 to 4
 * 
 * Dooropener activator
 * 
 * pin 6 of the UNO (defined by DOORPIN)
 * 
 * =====================================================================================
 * USAGE
 * =====================================================================================
 * If this is the first time the program ever runs on this board, you need to clear the EEPROM count, as we 
 * do NOT know what is currently in the length location. Enter the InitPromCode (defined below) on the 
 * keypad to set the length to zero. 
 * 
 * TO prevent an EEPROM overrun, the maximum size of the EEPROM can be set with MAXPOSTEEPROM.
 * 
 * You first need to pair a NFC card in the system. Enter the PairingCode (defined below) and 
 * then hold the card for the reader.
 * 
 * If you set TriggerMoment to EITHER. you can open the door with EITHER a valid NFC card or 
 * valid access password AccessCode (defined below). 
 * 
 * If you set TriggerMoment to BOTH. you can only open the door with BOTH a valid NFC card AND 
 * valid access password AccessCode (defined below). 
 * 
 * You can toggle between BOTH and EITHER without recompile. Enter the TogglePswCode (defined below) 
 * and if the TriggerMoment was BOTH it will become EITHER and the other way around.
 * 
 * The access password AccessCode (defined below) can be entered from remote over the serial port to 
 * open the door as well.
 * 
 * A new entry access code can be changed without recompile by entering the ChangePswCode (defined below). 
 * you will be prompted to enter a new access password from the keypad. 
 * You get 5 seconds to start entering the new password.
 * If that password is already use, the operation will cancel. Else you will be asked to enter '#' 
 * to confirm the new password
 * 
 * By setting #define DEBUG 0 to 1 you will get debug messages on your serial line.
 * 
 * 
 * ==============================================================================================
 * Extra libraries needed :
 * ==============================================================================================
 * https://github.com/Chris--A/Keypad
 * http://playground.arduino.cc/Code/Keypad 
 * 
 * https://www.makerguides.com/character-i2c-lcd-arduino-tutorial/
 * https://www.arduinolibraries.info/libraries/liquid-crystal-i2-c
 * https://github.com/marcoschwartz/LiquidCrystal_I2C 
 * 
 * https://playground.arduino.cc/Learning/MFRC522/
 * https://github.com/miguelbalboa/rfid
 * 
 * 
 * Version 1.0.2 / June 2020
 * - remove multiple PCD_Init()
 * 
 * Version 1.1.0 / June 2020
 *  - added storing new entry passwrd in EEPROM
 *  - added storing BOTH/ EITHER in EEPROM
 *  - KEYPAD 4 X 4 setting
 *  
 * Please be aware that I have NOT written much of the code. It is coming from different sources 
 * from Internet, developed by others. I do not know who or where the real sources are coming 
 * from. Hence I do not claim copyright or apply a license. 
 * This is grapware… for you to play around and enjoy. 
 * No Support, no warranty, no obligations. Just source code as-is !
 * 
 */
 
//=======================================================================
// SEE THE DOCUMENT IN THE SAME FOLDER AS THE SKETCH FOR MORE INFORMATION
//=======================================================================

#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

//=================================================================
// passwords 
// can be a maximum length of 10 digits BUT all MUST have the SAME length
//================================================================
String AccessCode="*123456#";    // entry access code 
String PairingCode="*654321#";   // pairing access code
String InitPromCode="##6655**";  // set EEPROM location to zero
String ChangePswCode="*9*9*57#"; // use to change for entry access code
String TogglePswCode="*#9*8*77"; // used to toggle between EITHER or BOTH door opening function

//=================================================================
// dooropening parameters
//================================================================
// set TriggerMoment to 
//  EITHER to open the door with EITHER valid NFC OR valid password from keypad
//  BOTH to open the door with BOTH valid NFC AND valid password from keypad
#define EITHER  1            
#define BOTH    2
int TriggerMoment = BOTH;

// Dooropener connection
#define DOORPIN 6

// Dooropener time (mS)
# define DOOR_OPEN_TIME 2000

//=================================================================
// NFC parameters
//================================================================
#define RST_PIN   9         // Configurable, see doc
#define SS_PIN    10        // Configurable, see doc
#define MAXPOSTEEPROM 1000  // Maximum size EEPROM in bytes

//=================================================================
// KEYPAD parameters
// select your keypad below EITHER 4 x 3 or 4 x 4 switches
//================================================================

//#define KEYPAD4x3 
#define KEYPAD4x4


#ifdef KEYPAD4x3        // if keypad is  4 x 3

#define numRows 4      // number of rows on the keypad
#define numCols 3      // number of columns on the keypad

//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]= {
{'1', '2', '3'},
{'4', '5', '6'},
{'7', '8', '9'},
{'*', '0', '#'}};

//Code that shows  the keypad connections to the arduino terminals
byte rowPins[numRows] = {5, A0, 7, 8}; // Rows 0 to 4
byte colPins[numCols] = {2, 3, 4};     // Columns 0 to 3

#elif defined(KEYPAD4x4)        // if pad is 4 x 4 

#define numRows 4      // number of rows on the keypad
#define numCols 4      // number of columns on the keypad

//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]= {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}};

//Code that shows  the keypad connections to the arduino terminals

byte rowPins[numRows] = {5, A0, 7, 8};      // Rows 0 to 4
byte colPins[numCols] = {2, 3, 4, A1};      // Columns 0 to 4

#else
#error "Please select one of the keypads."
#endif

//=================================================================
// DEBUG parameter (set to 1 for debug serial messages)
//================================================================
#define DEBUG 1

/////////////////////////////////////////////////////////
// NO CHANGES NEEDED BEYOND THIS POINT                 //
/////////////////////////////////////////////////////////

//==============================================
// GLOBAL VARIABLES
//=============================================

int OpenDoorState=0;   // needed to track for BOTH NFC and keypad
int state_bt=0;        // Serial input state
int stare=0;           // machine state for NFC / keypad
byte CODE[10];         // store the NFC code
byte AUX[10];          // used for compare NFC code

//==============================================
// Initialize
//=============================================
//nfc
MFRC522 mfrc522(SS_PIN, RST_PIN);      // Create MFRC522 instance
MFRC522::MIFARE_Key key;

//lcd
LiquidCrystal_I2C  lcd(0x27,16,2);

//initialize an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

//==============================================
// Start of program
//=============================================
void setup() {
  
  pinMode(DOORPIN,OUTPUT);            // door opener
  digitalWrite(DOORPIN,LOW);
  
  Serial.begin(9600);    // Initialize serial communications with the PC

  if(DEBUG == 1){
    while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    Serial.println(F("Provide your input?:"));
  }
  
  //nfc
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 card

  // set LCD
  lcd.init();
  lcd.backlight();
  blocked(0);

  // check for overwrite password or security level
  OverWriteCheck();

  
  // check passwords length
  if (Validate_passwrds() < 0){
    if (DEBUG == 1){
       Serial.println(F("Passwords are NOT same length or longer than 10 characters"));
    }
    
    //freeze
    for(;;);
  }
}

//=======================================================
// Check on password length max 10 and all the same length
//=======================================================
int Validate_passwrds()
{
  byte a = AccessCode.length();
  if ( a > 10) return(-1);    //access code > is to long

  if (PairingCode.length() != a) return(-1);  // if NOT same length
  if (InitPromCode.length() != a) return(-1);
  if (ChangePswCode.length() != a) return(-1);
  if (TogglePswCode.length() != a) return(-1);

  return(0);
}

//==============================================
// Print LCD blocked after delay
//=============================================
void blocked(unsigned long t)
{
  if (t) delay(t);
  
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print(F("BLOCKED"));
}

//==============================================
// extract NFC info
//=============================================
void  ExtractNFC(){
  for (byte i =0; i<(mfrc522.uid.size); i++) {
    CODE[i]=mfrc522.uid.uidByte[i];        
  }
  
  if(DEBUG == 1){
    Serial.print(F("CODE received: "));
    Serial.print(CODE[0],HEX);
    Serial.print(" ");
    Serial.print(CODE[1],HEX);
    Serial.print(" ");
    Serial.print(CODE[2],HEX);
    Serial.print(" ");
    Serial.println(CODE[3],HEX);
  }
}

//==============================================
// add a new card in EEPROM (if not already there)
//=============================================
void pairNFC() {
  
  if(DEBUG == 1){
    Serial.print(F("CODE to pair: "));
    Serial.print(CODE[0],HEX);
    Serial.print(" ");
    Serial.print(CODE[1],HEX);
    Serial.print(" ");
    Serial.print(CODE[2],HEX);
    Serial.print(" ");
    Serial.println(CODE[3]);
  }
  
  // check CODE is NOT already in
  for(int i=1;i<=EEPROM.read(0);i++) {

    switch(i%4){
      case 1 :{AUX[0]=EEPROM.read(i); break;}
      case 2 :{AUX[1]=EEPROM.read(i); break;}
      case 3 :{AUX[2]=EEPROM.read(i); break;}
      case 0 :{AUX[3]=EEPROM.read(i); break;}
    }

    if(i%4 == 0) {
      if(AUX[0]==CODE[0] && AUX[1]==CODE[1] && AUX[2]==CODE[2] && AUX[3]==CODE[3] ){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(F("CODE ALREADY IN"));
        lcd.setCursor(0,1);
        lcd.print(F("SYSTEM"));
        return;
      }
    }
  }

  // when we get here, CODE is NOT already in system    
  // get current last postion in EEPROM 
  int ttt=EEPROM.read(0);

  // check on Maximum size EEPROM
  if (ttt + 4 > MAXPOSTEEPROM){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("EEPROM IS FULL"));
    return;
  }
 
  // write to EEPROM
  EEPROM.write(ttt+16+1,CODE[0]);
  EEPROM.write(ttt+16+2,CODE[1]);
  EEPROM.write(ttt+16+3,CODE[2]);
  EEPROM.write(ttt+16+4,CODE[3]);

  // save new length
  ttt=ttt+4;
  EEPROM.write(0,ttt);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("CODE PAIRED"));
  
  if(DEBUG == 1){
    Serial.print(F("CODE PAIRED: "));
    Serial.print(CODE[0],HEX);
    Serial.print(" ");
    Serial.print(CODE[1],HEX);
    Serial.print(" ");
    Serial.print(CODE[2],HEX);
    Serial.print(" ");
    Serial.println(CODE[3]);
  }
}

//==============================================
// reset EEPROM (as we do not know what is in EEPROM count if we start from scratch)
// EEPROM layout
// address 0        length of NFC bytes stored
// address 1 - 12d  overwrite password (max length is 10 + zero)
// address 14d      overwrite BOTH or EITHER
// address 15d      RFU
// address 16d      start of NFC bytes store
//=============================================
void InitProm()
{
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print(F("Clear EEPROM"));

  // reset length
  EEPROM.write(0,0);

  // reset passwd
  for (byte x = 1; x < 13; x++) EEPROM.write(x,0);

  // reset Either or BOTH security level
  EEPROM.write(14,0);
  
  blocked(2000);
}

//==============================================
// Check overwrite of access password and security level
// These values can be changed with a special password while active. If someone had applied change and
// there was a power failure, you want to use the changed password / security level are reboot.
//
// EEPROM layout
// address 0        length of NFC bytes stored (starting address 16d)
// address 1 - 13d  overwrite password (max length is 12)
// address 14d      overwrite BOTH or EITHER
// address 15d      RFU
// address 16d      start of NFC bytes store
//=============================================

void OverWriteCheck()
{
  char NewPswd[13];
  byte i;
  
  // if new entry passwd was entered
  if (EEPROM.read(1)!= 0){
    i = 0;
    do {
      NewPswd[i++] = EEPROM.read(i+1);
    } while( NewPswd[i -1] != 0x0 && i < 10);

    NewPswd[i] = 0x0;
    String str(NewPswd);
    AccessCode = str;

    if(DEBUG == 1){
      Serial.print("Password read from EEPROM: ");
      Serial.println(AccessCode);
    }
  }

  // changed security
  if (EEPROM.read(14)!= 0){
    TriggerMoment = EEPROM.read(14);

    if(DEBUG == 1){
      Serial.print("Security level read from EEPROM: ");
      if(TriggerMoment == BOTH) Serial.println("BOTH");
      else Serial.println("EITHER");
    }
  }
}

//==============================================
//check for CODE match with stored in EEPROM
//=============================================
boolean validateNFC(){

   // read complete EEPROM for match
   for(int i=16; i<=16 + EEPROM.read(0); i++) {

    switch(i%4){
      case 1 :{AUX[0]=EEPROM.read(i); break;}
      case 2 :{AUX[1]=EEPROM.read(i); break;}
      case 3 :{AUX[2]=EEPROM.read(i); break;}
      case 0 :{AUX[3]=EEPROM.read(i); break;}
    }

    if(i%4 == 0) {  // check for match
      if( AUX[0]==CODE[0] && AUX[1]==CODE[1] && AUX[2]==CODE[2] && AUX[3]==CODE[3]) {
        return(true);
      }
    }
  }
  
  return(false);
}

//==============================================
// check code match
//=============================================
int ComparePswd(String a) {
  if(a.equals(AccessCode)) return 1;
  else if(a.equals(PairingCode)) return 2;
  else if(a.equals(InitPromCode)) return 3;
  else if(a.equals(ChangePswCode)) return 4;
  else if(a.equals(TogglePswCode)) return 5;
  else return 0;
}

//==============================================
// read keypad information
//=============================================
String GetKeyPad(char x, boolean show) {
  char vec[10];
  int i;

  vec[0]=x;
  
  if(DEBUG == 1){
    Serial.print(F("got on keypad "));
    Serial.println(vec[0]);
  }
  
  lcd.setCursor(0,0);
  lcd.clear();
  
  if(show) lcd.print(x);      // show real digit or 'X'
  else lcd.print('X');    
    
  for(i=1; i<PairingCode.length(); i++) {    // get complete code
    lcd.setCursor(0,1);
    lcd.print("Need ");
    lcd.print(PairingCode.length()-i);
    lcd.print(" digits");

    vec[i]=myKeypad.waitForKey();
    
    if(DEBUG == 1){
      Serial.print(F("got on keypad "));
      Serial.println(vec[i]);
    }
    
    lcd.setCursor(i,0);
    if(show) lcd.print(vec[i]);    
    else lcd.print('X');    
  }

  vec[PairingCode.length()]=0x0;
  String str(vec);
  return str;
}

//==============================================
// Open the door
//=============================================
void open_door()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("UNLOCKED"));

  digitalWrite(DOORPIN,HIGH); // open door
  delay(DOOR_OPEN_TIME);
  digitalWrite(DOORPIN,LOW);

  blocked(0);
}

//==============================================
// Set new access passwd
//=============================================
void SetNewPswd()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Enter new pswd:"));

  unsigned long now = millis();

  // timeout for 5 seconds
  while(millis() - now < 5000) {
    
    // get new access code
    char c = myKeypad.getKey();
    if(c != NO_KEY) {
    
       // extract the keypad code
       String CodeCurrent=GetKeyPad(c, true);

       int A = ComparePswd(CodeCurrent);

       // Match to any of the existing passwords ?
       if(A != 0) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(F("Existing pswd"));        
          lcd.setCursor(0,1);
          lcd.print(F("cancelled"));
          blocked(2000);
          return;
       }

       // check for maximum length
       byte j = AccessCode.length();

       if (j > 10){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(F("Password too LONG")); 
          blocked(2000);
          return;           
       }
       
       // confirm new password
       lcd.setCursor(0,1);
       lcd.print(F("Press # to use"));         

       do {
        c = myKeypad.getKey();
       } while (c == NO_KEY);

       // update the password
       if(c == '#') {
          
          AccessCode = CodeCurrent;
          
          // store EEPROM (start address EEPROM 1)
          for (byte x = 0; x < j; x++) EEPROM.write(x+1,AccessCode[x]);
          
          if(DEBUG == 1) {
            Serial.print(F("New Access password"));
            Serial.println(AccessCode);
          }
          
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(F("New pswd set")); 
          blocked(2000);
          return;
       }
    }
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("cancelled"));
  blocked(2000);
}

//==============================================
// Toggle between EITHER or BOTH door opening
//=============================================
void ToggleOpening()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Door Triggers"));
  lcd.setCursor(0,1);
  if (TriggerMoment == EITHER) {
    lcd.print(F("now on BOTH"));
    TriggerMoment = BOTH;
  }
  else {
    lcd.print(F("now on EITHER"));
    TriggerMoment = EITHER;
  }
  // store in EEPROM (address 14)
  EEPROM.write(14,TriggerMoment);
  
  blocked(2000);
}
//==============================================
// Main_loop
//=============================================
void loop() {

  //Start BT autentification
  if (Serial.available()) {

    char c = Serial.read();
      
    // do we have match 
    if (AccessCode[state_bt] == c) {
      state_bt++;
      
      // does the complete password match
      if (state_bt == AccessCode.length()) {
        open_door();
        state_bt = 0;   // reset for next round
      }
    }
    else
      state_bt = 0;    // reset wrong code
  }  // end serialavailable()

  switch(stare){

    // read and validate NFC or Keypad
    case 0:
     {
         // check for NFC card
       if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ){

         // extract CODE
         ExtractNFC();

         // validate CODE against EEPROM
         if(validateNFC()) {

            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("VALID NFC CODE"));
            
            if (TriggerMoment == EITHER)  stare = 1;     // open the door in the next loop
            else if (OpenDoorState == 2)  stare = 1;     // if password was also detected now open the door
            else {
              OpenDoorState = 1;                         // indicate the card was detected
              lcd.setCursor(0,1);
              lcd.print(F("Need Password"));
             }

            delay(1000);
            return;
         }
         else {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("INVALID NFC CODE"));
            blocked(2000);
            OpenDoorState = 0;                           // indicate the No valid card was detected
            return;
         }
       }

       // check keypad for access code
       char c = myKeypad.getKey();
       if(c != NO_KEY) {

          // extract the keypad code
          String CodeCurrent=GetKeyPad(c, false);

          int A = ComparePswd(CodeCurrent);

          // None match to any of the passwords
          if(A == 0) {
            lcd.clear();
            lcd.print(F("INVALID CODE"));
            blocked(2000);
            OpenDoorState = 0;                          // indicate the No valid password was detected
            return;
          }

          // entry access code
          else if(A == 1) {
            lcd.setCursor(0,0);
            lcd.clear();
            lcd.print(F("VALID CODE"));

            if (TriggerMoment == EITHER)  stare = 1;    // open the door in the next loop
            else if (OpenDoorState == 1)  stare = 1;    // if NFC was also detected open the door
            else {
              OpenDoorState = 2;                        // indicate the Passwd was detected
              lcd.setCursor(0,1);
              lcd.print(F("Need card"));
            }
            delay(1000);
            return;
          }
          
          // pairing access code
          else if(A == 2) {
            stare=2;               // perform pairing in next loop
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Pairing...");
            return;
          }
          
          // reset EEprom counter
          else if (A == 3) {
            InitProm();
          }
          
          // Set new entry access password
          else if (A == 4) {
            SetNewPswd();
          }

          // Toggle between EITHER or BOTH
          else if (A == 5) {
            ToggleOpening();
          }
       } // keypad check
       break;
     }
      
    // open the door
    case 1:
        open_door();
        stare=0;                    // reset to wait for NFC or KEYPAD
        OpenDoorState = 0;          // indicate NO valid NFC nor Passwrd
        break;
     
    // perform pairing
    case 2:
        //mfrc522.PCD_Init();
        
        lcd.setCursor(0,1);
        lcd.print(F("Need card"));
        
        if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ){
          
          // extract card
          ExtractNFC();

          // store in EEPROM
          pairNFC();
          stare=0;                  // reset to wait for NFC or KEYPAD
          OpenDoorState = 0;        // indicate NO valid NFC nor Password
          blocked(2000);
        }
        break;
        
    default :
        if(DEBUG == 1) {
          Serial.print(F("Default. you should never get here.  stare = "));
          Serial.println(stare,HEX);
        }
        break;
    }
}
