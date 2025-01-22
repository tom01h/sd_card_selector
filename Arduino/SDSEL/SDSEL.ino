#define SELECT 14 // Card HIGH:PI / LOW:USB 
#define DETECT 33 // Card LOW:DETECT 
#define PSW 15 // PSW Switch 
#define PWRDET 32 // Power Detect

#include<ctype.h>

__xdata char recvStr[64];
uint8_t recvStrPtr = 0;
bool stringComplete = false;
uint16_t echoCounter = 0;

enum {S_USB=0, S_PI_OFF, S_PI_ON, S_INVALID};
uint8_t status=S_USB;
//uint8_t status=S_PI_OFF;

void prompt(){
  switch(status){
    case S_USB: USBSerial_print("USB> "); break;
    case S_PI_OFF: USBSerial_print("Pi Off> "); break;
    case S_PI_ON: USBSerial_print("Pi On> "); break;
    case S_INVALID: USBSerial_print("INVALID> "); break;
  }
  USBSerial_flush();
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SELECT, OUTPUT);
  pinMode(DETECT, OUTPUT_OD);
  pinMode(PSW, OUTPUT_OD);
  pinMode(PWRDET, INPUT);

  digitalWrite(PSW, HIGH);
  
  digitalWrite(SELECT, LOW);
  delay(100);
  digitalWrite(DETECT, LOW);

  while (!USBSerial()) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  USBSerial_println("");
  USBSerial_println("Raspberry Pi Remote");
  USBSerial_println("USB Card Reader");
  if(digitalRead(PWRDET)==0){
    status=S_USB;
  }else{
    status=S_INVALID;
  }
  //if(digitalRead(PWRDET)==0){
  //  status=S_PI_OFF;
  //}else{
  //  status=S_PI_ON;
  //}
  prompt();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (stringComplete) {
    if(digitalRead(PWRDET)==0){
      switch(status){
        case S_USB: status=S_USB; break;
        case S_PI_OFF: status=S_PI_OFF; break;
        case S_PI_ON: status=S_PI_OFF; break;
        case S_INVALID: status=S_USB; break;
      }
    }else{
      switch(status){
        case S_USB: status=S_INVALID; break;
        case S_PI_OFF: status=S_PI_ON; break;
        case S_PI_ON: status=S_PI_ON; break;
        case S_INVALID: status=S_INVALID; break;
      }
    }
    
    USBSerial_println("");
    if(strcmp(recvStr, "usb")==0){
      if(status==S_PI_OFF){
        status=S_USB;
        USBSerial_println("USB Card Reader");
        digitalWrite(SELECT, LOW);
        delay(100);
        digitalWrite(DETECT, LOW);
      }else{
        USBSerial_println("Invalid Command");
      }
    }else if(strcmp(recvStr, "rpi")==0){
      if(status==S_USB){
        status=S_PI_OFF;
        USBSerial_println("Raspberry Pi");
        digitalWrite(DETECT, HIGH);
        delay(100);
        digitalWrite(SELECT, HIGH);
      }else{
        USBSerial_println("Invalid Command");
      }
    }else if(strcmp(recvStr, "pon")==0){
      if(status==S_PI_OFF){
        //status=S_PI_ON;
        USBSerial_println("Power ON");
        digitalWrite(PSW, LOW);
        delay(300);
        digitalWrite(PSW, HIGH);
      }else{
        USBSerial_println("Invalid Command");
      }
    }else if(strcmp(recvStr, "shutdown")==0){
      if(status==S_PI_ON || status==S_INVALID){
        status=S_USB;
        USBSerial_println("Shutdown");
        digitalWrite(PSW, LOW);
        delay(6000);
        digitalWrite(PSW, HIGH);
        delay(100);
        digitalWrite(SELECT, LOW);
        delay(100);
        digitalWrite(DETECT, LOW);
      }else{
        USBSerial_println("Invalid Command");
      }
    }else if(strcmp(recvStr, "help")==0){
      USBSerial_println("rpi: Raspberry Pi");
      USBSerial_println("usb: USB Card Reader");
      USBSerial_println("pon: Power ON");
      USBSerial_println("shutdown: Shutdown");
    }else{
      USBSerial_println("Undefined Command");
    }

    prompt();

    stringComplete = false;
    recvStrPtr = 0;
  }

  if (USBSerial_available()) {
    char serialChar = USBSerial_read();
    if ((serialChar == '\n') || (serialChar == '\r') ) {
      recvStr[recvStrPtr] = '\0';
      stringComplete = true;
    } else {
      recvStr[recvStrPtr] = tolower(serialChar);
      recvStrPtr++;
      USBSerial_write(serialChar);
      if (recvStrPtr == 63) {
        recvStr[recvStrPtr] = '\0';
        stringComplete = true;
      }
    }
    USBSerial_flush();
  }
}
