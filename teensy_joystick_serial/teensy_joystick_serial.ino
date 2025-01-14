//---set usb type as flight sim controls + joystick


#include <seesaw_spectrum.h>
#include <seesaw_servo.h>
#include <seesaw_neopixel.h>
#include <seesaw_motor.h>
#include <Adafruit_TFTShield18.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_miniTFTWing.h>
#include <Adafruit_Crickit.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_NeoKey_1x4.h>
#include <Arduino.h>
#include <varObj.h>
#include <ods_util.h>
#include <serialPrint.h>
#include <PWMServo.h>

#define SER_DLY           250
#define LOOP_DLY          10
#define DEF_BAUD          115200

#define NUM_PIXELS        1

#define AXIS_0_PIN  14 // A0
#define AXIS_1_PIN  15 // A1
#define AXIS_2_PIN  16 // A2



#define BTN0_PIN    2
#define BTN1_PIN    3
#define BTN2_PIN    4
#define BTN3_PIN    5
#define BTN4_PIN    6
#define BTN5_PIN    7
#define BTN6_PIN    8

#define NEOPIXEL_PIN    A3
#define NEOKEY_BTN_PIN  A4


#define LED_MODULO  25
#define SER_OUT_MODULO  5

#define AIN_MAX 1023
#define AIN_MID 512


#define EXPO_MODE         EXP_MD_BILATERAL
#define PITCH_EXPO_PARAM  15
#define ROLL_EXPO_PARAM   17
#define YAW_EXPO_PARAM    16

#define Y_DIM 2 //number of rows of keys
#define X_DIM 4 //number of columns of keys

enum opModes {
  OP_MD_NONE,
  OP_MD_ROLL,
  OP_MD_PITCH,
  OP_MD_YAW,
  NUM_OP_MODES
};

String opMdStrs[] = {
  "None",
  "Adj Roll",
  "Adj Pitch",
  "Adj Yaw"
};

int opMd;
bool serialOk;
int tmr;
uint32_t iCount;
bool prntHex;

int ain01;

uint8_t neopixelBtns;

bool neoPxlBtn0;
bool neoPxlBtn1;
bool neoPxlBtn2;
bool neoPxlBtn3;

bool btn0;
bool btn1;
bool btn2;
bool btn3;
bool btn4;
bool btn5;
bool btn6;

int enYawExpoInc;
int enYawExpoDec;
int enRollExpoInc;
int enRollExpoDec;
int enPitchExpoInc;
int enPitchExpoDec;


bool btn0Shadow;
bool btn1Shadow;
bool btn2Shadow;
bool btn3Shadow;
bool btn4Shadow;
bool btn5Shadow;
bool btn6Shadow;

uint8_t sld_led0;
uint8_t sld_led1;

uint8_t btn0_cnt;
uint8_t btn1_cnt;
uint8_t btn2_cnt;
uint8_t btn3_cnt;
uint8_t btn4_cnt;
uint8_t btn5_cnt;
uint8_t btn6_cnt;

varObj voRoll;
varObj voPitch;
varObj voYaw;
varObj vo03;

int btnCount;

bool useVJoy;
bool useServo;
bool useThrottleServo;
bool printBtns;
bool camPitchEn;

float camPitch;
float camPitchGain;


Adafruit_NeoKey_1x4 neoKey;

PWMServo svo;
int svoPos;


//=============================================================================
void setup() {
  svo.attach(2, 1000, 2000);
  ain01 = 0;
  svoPos = 90;
  opMd = OP_MD_NONE;

  serialOk = false;
  iCount = 0;
  tmr = 25;
  prntHex = false;
  useVJoy = false;
  printBtns = false;
  btnCount = 0;

  btn0 = false;
  btn1 = false;
  btn2 = false;
  btn3 = false;
  btn4 = false;
  btn5 = false;
  btn6 = false;

  enYawExpoInc = 0;
  enYawExpoDec = 0;
  enRollExpoInc = 0;
  enRollExpoDec = 0;
  enPitchExpoInc = 0;
  enPitchExpoDec = 0;

  neopixelBtns = 0;
  neoPxlBtn0 = false;
  neoPxlBtn1 = false;
  neoPxlBtn2 = false;
  neoPxlBtn3 = false;

  btn0Shadow = false;
  btn1Shadow = false;
  btn2Shadow = false;
  btn3Shadow = false;
  btn4Shadow = false;
  btn5Shadow = false;
  btn6Shadow = false;

  btn0_cnt = 0;
  btn1_cnt = 0;
  btn2_cnt = 0;
  btn3_cnt = 0;
  btn4_cnt = 0;
  btn5_cnt = 0;
  btn6_cnt = 0;

  camPitchEn = false;

  camPitch = 0.0;
  camPitchGain = 1.0;

  Serial.begin(DEF_BAUD);

  while (tmr > 0 && serialOk) {
    ledToggle();
    if (Serial)
      serialOk = true;

    delay(SER_DLY);

    tmr--;
  }

  Serial.println("Serial Port OK");

  Serial.println("Init AIN channels");
  pinMode(AXIS_0_PIN, INPUT);
  pinMode(AXIS_1_PIN, INPUT);
  pinMode(AXIS_2_PIN, INPUT);

  Serial.println("Init DIN channels");
  //pinMode(NEOKEY_BTN_PIN, INPUT_PULLUP);
  pinMode(BTN0_PIN, INPUT_PULLUP);
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);
  pinMode(BTN4_PIN, INPUT_PULLUP);
  pinMode(BTN5_PIN, INPUT_PULLUP);
  pinMode(BTN6_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Init neoKey");
  if (!neoKey.begin(0x30)) {
    Serial.println("Could not start neoKey!!!");
    //while (1) delay(10);
  }
  else
    Serial.println("Neokey init good");

  ledToggle();
  delay(1000);

  for (uint16_t i = 0; i < neoKey.pixels.numPixels(); i++) {
    neoKey.pixels.setPixelColor(i, Wheel(map(i, 0, neoKey.pixels.numPixels(), 0, 255)));
    neoKey.pixels.show();
    delay(50);
  }
  for (uint16_t i = 0; i < neoKey.pixels.numPixels(); i++) {
    neoKey.pixels.setPixelColor(i, 0x000000);
    neoKey.pixels.show();
    delay(50);
  }


  Serial.println("Init varObj instances");
  voRoll = varObj(V_TYP_UINT10, true);
  voRoll.expoMode = EXPO_MODE;
  voRoll.setExpo(ROLL_EXPO_PARAM);

  voPitch = varObj(V_TYP_UINT10, false);
  voPitch.expoMode = EXPO_MODE;
  voPitch.setExpo(PITCH_EXPO_PARAM);

  voYaw = varObj(V_TYP_UINT10, true);
  voYaw.expoMode = EXPO_MODE;
  voYaw.setExpo(YAW_EXPO_PARAM);

  //vo03 = varObj(V_TYP_UINT10, false);
  //vo03.expoMode = EXPO_MODE;

  Serial.println("voRoll:" + voRoll.getBC());
  Serial.println("voPitch:" + voPitch.getBC());
  Serial.println("voYaw:" + voYaw.getBC());
  Serial.println("vo03:" + vo03.getBC());


  ledToggle();
  delay(1000);
  Serial.println("Setup done");
}

//=================================================================================================
void handleSerIn() {
  String inStr = recvWithEndMarker();
  if(inStr > "") {

    serPrntNL(handleFloatParam(&camPitchGain, "cpg", inStr, 1.0, 0));

    if (inStr == "vj") {
      useVJoy = !useVJoy;
      serPrnt("ic:" + String(iCount) + ":");
      serPrntNL("toggle between serial and joystick mode");
    }

    else if (inStr == "h") {
      prntHex = !prntHex;
      serPrnt("ic:" + String(iCount) + ":");
      serPrntNL("toggle print in hex");
    }
    else if (inStr == "s") {
      useServo = !useServo;
      if(useServo)
        useThrottleServo = false;
      serPrnt("ic:" + String(iCount) + ":");
      serPrntNL("toggle servo out en");
    }
    else if (inStr == "th") {
      useThrottleServo = !useThrottleServo;

      if(useThrottleServo)
        useServo = false;
      serPrnt("ic:" + String(iCount) + ":");
      serPrntNL("toggle throttle servo out");
    }
    else {
      serPrnt("ic:" + String(iCount) + ":");
      serPrnt(":" + inStr);
      serPrntNL(":unhandled cmd");
    }

  }
}

//=================================================================================================
void taskSerialOut() {
  String tmpStr = "";

  if (iCount % SER_OUT_MODULO == 0){

    Serial.print(F("tjs iC:"));
    Serial.print(String(iCount));

    Serial.print(" opM:" + opMdStrs[opMd]);



    switch (opMd) {
      case OP_MD_NONE:
      default:
        Serial.print(" b:" + (String)btn0 + (String)btn1 + (String)btn2 + (String)btn3);
        Serial.print((String)btn4 + (String)btn5 + (String)btn6); 
        Serial.print(" N:" + (String)neopixelBtns);
        Serial.print(">" + (String)neoPxlBtn0);
        Serial.print(":" + (String)neoPxlBtn1);
        Serial.print(":" + (String)neoPxlBtn2);
        Serial.print(":" + (String)neoPxlBtn3);
        Serial.print(F(" Roll:"));
        Serial.print(voRoll.getVal());

        Serial.print(F(" Pitch:"));
        Serial.print(voPitch.getVal());

        Serial.print(F(" Yaw:"));
        Serial.print(voYaw.getVal());
        break;

      case OP_MD_ROLL:
        Serial.print(F(" Roll:"));
        Serial.print(" R+:");
        Serial.print(enRollExpoInc);
        Serial.print(" R-:");
        Serial.print(enRollExpoDec);

        Serial.print(F(" v:"));
        Serial.print(voRoll.getVal());
        Serial.print(F(" expo:"));
        Serial.print(voRoll.expoParam);

        break;

      case OP_MD_PITCH:
        Serial.print(F(" Pitch:"));
        Serial.print(" P+:");
        Serial.print(enPitchExpoInc);
        Serial.print(" P- :");
        Serial.print(enPitchExpoInc);

        Serial.print(F(" v:"));
        Serial.print(voPitch.getVal());
        Serial.print(F(" expo:"));
        Serial.print(voPitch.expoParam);
        break;

      case OP_MD_YAW:
        Serial.print(F(" Yaw:"));
        Serial.print(" Y+:");
        Serial.print(enYawExpoInc);
        Serial.print(" Y-:");
        Serial.print(enYawExpoDec);

        Serial.print(enYawExpoDec);
        Serial.print(F(" v:"));
        Serial.print(voYaw.getVal());
        Serial.print(F(" expo:"));
        Serial.print(voYaw.expoParam);
        break;
    }

    ////Serial.printf(" voPitch.getNorm: %5.2f", voPitch.getNorm());
    //Serial.printf(" cpGain: %5.2f", camPitchGain);
    //Serial.printf(" cpEn: %d", (int)camPitchEn);
    //Serial.printf(" cp: %5.2f", camPitch);


    serPrntNL();

    }
  }

//=============================================================================
void taskTelemOut() {
  String tmpStr = "";
  if (iCount % 1 == 0) {
    tmpStr += "A55A";
    tmpStr += "_a0:" + String(voRoll.getVal());
    tmpStr += "_a1:" + String(voPitch.getVal());
    tmpStr += "_a2:" + String(voYaw.getVal());
    //tmpStr += "_a3:" + String(vo03.getVal());
    tmpStr += "_b0:" + String(btn0);
    tmpStr += "_b1:" + String(btn1);
    tmpStr += "_b2:" + String(btn2);
    tmpStr += "_b3:" + String(btn3);
    tmpStr += "_b4:" + String(btn4);
    tmpStr += "_b5:" + String(btn5);
    tmpStr += "_b6:" + String(btn6);
    tmpStr += "B66B";
    Serial.println(tmpStr);
  }
}

//=============================================================================
void taskDigRead() {

  btn0 = digitalRead(BTN0_PIN) ? false : true;
  btn1 = digitalRead(BTN1_PIN) ? false : true;
  btn2 = digitalRead(BTN2_PIN) ? false : true;
  btn3 = digitalRead(BTN3_PIN) ? false : true;
  btn4 = digitalRead(BTN4_PIN) ? false : true;
  btn5 = digitalRead(BTN5_PIN) ? false : true;
  btn6 = digitalRead(BTN6_PIN) ? false : true;

  neopixelBtns = neoKey.read();
  
  neoPxlBtn0 = neopixelBtns == 1 ? true : false;
  neoPxlBtn1 = neopixelBtns == 2 ? true : false;
  neoPxlBtn2 = neopixelBtns == 4 ? true : false;
  neoPxlBtn3 = neopixelBtns == 8 ? true : false;

  camPitchEn = btn0;


  btn0Shadow = btn0;
  btn1Shadow = btn1;
  btn2Shadow = btn2;
  btn3Shadow = btn3;
  btn4Shadow = btn4;
  btn5Shadow = btn5;
  btn6Shadow = btn6;
}

//=============================================================================
void taskAnalogRead() {

  voRoll.pushVal(analogRead(AXIS_0_PIN));
  voPitch.pushVal(analogRead(AXIS_1_PIN));

  voYaw.pushVal(analogRead(AXIS_2_PIN));
  //vo03.pushVal(analogRead(AXIS_3_PIN));
}

//=============================================================================
void taskDigWrite() {
}

//=============================================================================
void taskAnalogWrite() {

}
//=============================================================================
void taskHandle_js_out() {
  Joystick.X(voRoll.getVal());
  Joystick.Y(voPitch.getVal());
  Joystick.Z(voYaw.getVal());
  Joystick.Zrotate(AIN_MID);
  //Joystick.Zrotate((int)camPitch);
  Joystick.slider(AIN_MID);
  Joystick.sliderLeft(AIN_MID);
  Joystick.sliderRight(AIN_MID);
  Joystick.sliderLeft(AIN_MID);
  Joystick.sliderRight(AIN_MID);



  Joystick.button(1, btn0);
  Joystick.button(2, btn1);

  Joystick.button(3, btn2);
  Joystick.button(4, btn3);
  Joystick.button(5, neopixelBtns);


}

//=============================================================================
void doMixing() {
  //static int tempPitch = 0;
  if (camPitchEn == true) {
    camPitch = camPitchGain * ((float)voPitch.getVal() - 512.0);
    camPitch += 512.0;
    //camPitch += camPitchGain * 512.0;
    //camPitch = tempPitch;
  }
  else
    camPitch = 0.0;



   if (useServo)
     svoPos = map((float)ain01, 215, 880, 10, 170);
   else if (useThrottleServo)
     svoPos = map((float)voRoll.getVal(), 215, 880, 10, 170);

  }

//=============================================================================
void taskOpMode() {
  static bool _neoPxlBtnShadow = false;

  if(iCount % 10 == 0){
    if (neoPxlBtn0 != _neoPxlBtnShadow && neoPxlBtn0)
      opMd++;

    if (opMd > OP_MD_YAW)
      opMd = OP_MD_NONE;

    _neoPxlBtnShadow = neoPxlBtn0;
  }


}

//=============================================================================
void taskNeoPixel() {

  for(int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
    neoKey.pixels.setPixelColor(i, 0x003f0000);

  switch (opMd) {
    case OP_MD_NONE:
    default:
      break;

    case OP_MD_ROLL:
      neoKey.pixels.setPixelColor(1, 0xFF00FF00);
      break;

    case OP_MD_PITCH:
      neoKey.pixels.setPixelColor(2, 0xFF00FF00);
      break;
  
    case OP_MD_YAW:
      neoKey.pixels.setPixelColor(3, 0xFF00FF00);
      break;
  }
  neoKey.pixels.show();
}

//=============================================================================
void taskExpoEdit() {
  switch (opMd) {
    case OP_MD_NONE:
    default:
      break;

    case OP_MD_ROLL:
      //Serial.println("tpe: ROll");
      if (voYaw.getVal() > 1020) {
        enRollExpoInc++;
        enRollExpoDec = 0;

        if(enRollExpoInc == 1)
        {
          voRoll.expoParam++;
          voRoll.setExpo();
        }
      }
      else if (voYaw.getVal() < 3) {
        enRollExpoDec++;
        enRollExpoInc = 0;

        if (enRollExpoDec == 1)
        {
          voRoll.expoParam--;
          voRoll.setExpo();
        }
      }
      else {
        enRollExpoInc = 0;
        enRollExpoDec = 0;
      }
      break;

    case OP_MD_PITCH:
      //----yaw max: inc pitch expo
      if (voYaw.getVal() > 1020) {

        enPitchExpoInc++;
        enPitchExpoDec = 0;

        if(enPitchExpoInc == 1){
          voPitch.expoParam++;
          voPitch.setExpo();
        }
      }
      //---yaw min: dec pitch expo
      else if (voYaw.getVal() < 3) {
        enYawExpoInc++;
        enYawExpoDec = 0;


        if(enPitchExpoDec == 1) {
          voPitch.expoParam--;
          voPitch.setExpo();
        }
      }
      //---everything else
      else {
        enPitchExpoInc = 0;
        enPitchExpoInc = 0;
      }
      break;

    case OP_MD_YAW:
      if (voYaw.getVal() > 1020) {
        enYawExpoInc++;
        enYawExpoDec = 0;

        if(enYawExpoInc == 1){
          voYaw.expoParam++;
          voYaw.setExpo();
        }
      }
      else if (voYaw.getVal() < 3) {
        enYawExpoDec = 0;
        enYawExpoInc++;
        
        if(enYawExpoDec == 1) {
          voYaw.expoParam--;
          voYaw.setExpo();
        }
      }
      break;
  }
}

//=============================================================================
void loop() {
  iCount++;
  if (iCount % LED_MODULO == 0)
    ledToggle();

  taskAnalogRead();
  taskDigRead();
  handleSerIn();
  taskOpMode();

  doMixing();

  taskExpoEdit();
  taskAnalogWrite();
  taskDigWrite();

  taskNeoPixel();

  if(useVJoy) {
    taskTelemOut();

  }
  else {
    taskSerialOut();
    taskHandle_js_out();
  }

  //svo.write(svoPos);
  // a brief delay, so this runs 20 times per second
  delay(LOOP_DLY);
}
