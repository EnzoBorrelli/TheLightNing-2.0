/*
----LightNing 2.0 code V1----

codigo de control del RC-car, comunicando el NODEMcu-esp32s con DualSHock4®

- ByEnder|2023

*/

#include <Bluepad32.h>

//Librerias propias
#include "Defines.h"
#include "motorClass.h"
#include "rgbColors.h"

#define DEBUG true
//constantes de giro
#define JOYSTICK_DEADZONE 70
#define PWM_LOST 50

// constante de velocidad
#define NORMAL_SPEED 200

//inicializacion de los objetos de los motores
Motor *LadoIZQ = new Motor(PWM_MOTOR_A1, PWM_MOTOR_A2, PIN_MOTOR_A1, PIN_MOTOR_A2, FREQ, RES);
Motor *LadoDER = new Motor(PWM_MOTOR_B1, PWM_MOTOR_B2, PIN_MOTOR_B1, PIN_MOTOR_B2, FREQ, RES);

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == nullptr) {
      Serial.printf("CALLBACK: Gamepad is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      GamepadProperties properties = gp->getProperties();
      Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n",
                    gp->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myGamepads[i] = gp;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
      "CALLBACK: Gamepad connected, but could not found empty slot");
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == gp) {
      Serial.printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
      myGamepads[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
      "CALLBACK: Gamepad disconnected, but not found in myGamepads");
  }
}

// Arduino setup function. Runs in CPU 1
void setup() {
  if (DEBUG) Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t *addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2],
                addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

// Arduino loop function. Runs in CPU 1
void loop() {
  // This call fetches all the gamepad info from the NINA (ESP32) module.
  // Just call this function in your main loop.
  // The gamepads pointer (the ones received in the callbacks) gets updated
  // automatically.
  BP32.update();
  // It is safe to always do this before using the gamepad API.
  // This guarantees that the gamepad is valid and connected.
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    GamepadPtr myGamepad = myGamepads[i];

    if (myGamepad && myGamepad->isConnected()) {
      myGamepad->setColorLED(white[0], white[1], white[2]);
      if (myGamepad->throttle()) {
        LadoIZQ->MotorUp(NORMAL_SPEED);
        LadoDER->MotorUp(NORMAL_SPEED);
        myGamepad->setColorLED(green[0], green[1], green[2]);
        if (DEBUG) Serial.println("pa lante");
      }
      if (myGamepad->brake()) {
        LadoIZQ->MotorDown(NORMAL_SPEED);
        LadoDER->MotorDown(NORMAL_SPEED);
        myGamepad->setColorLED(orange[0], orange[1], orange[2]);
        if (DEBUG) Serial.println("pa atras");
      }
      if (myGamepad->axisX()) {
        if (myGamepad->axisX() < -JOYSTICK_DEADZONE) {
          LadoIZQ->MotorUp(NORMAL_SPEED - PWM_LOST);
          LadoDER->MotorUp(NORMAL_SPEED);
          myGamepad->setColorLED(blue[0], blue[1], blue[2]);
          if (DEBUG) Serial.println("giro izq");
        }
        if (myGamepad->axisX() > JOYSTICK_DEADZONE) {
          LadoIZQ->MotorUp(NORMAL_SPEED);
          LadoDER->MotorUp(NORMAL_SPEED - PWM_LOST);
          myGamepad->setColorLED(cyan[0], cyan[1], cyan[2]);
          if (DEBUG) Serial.println("giro der");
        }
      }
      if (myGamepad->buttons() == 0x0010) {
        LadoIZQ->MotorDown(NORMAL_SPEED);
        LadoDER->MotorUp(NORMAL_SPEED);
        myGamepad->setColorLED(purple[0], purple[1], purple[2]);
        if (DEBUG) Serial.println("rotacion izq");
      }
      if (myGamepad->buttons() == 0x0020) {
        LadoIZQ->MotorUp(NORMAL_SPEED);
        LadoDER->MotorDown(NORMAL_SPEED);
        myGamepad->setColorLED(violet[0], violet[1], violet[2]);
        if (DEBUG) Serial.println("rotacion der");
      }
      if (myGamepad->x()) {
        LadoIZQ->MotorStop();
        LadoDER->MotorStop();
        myGamepad->setColorLED(red[0], red[1], red[2]);
      }
    }
  }

  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  // vTaskDelay(1);
  delay(150);
}
