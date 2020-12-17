/**
 *******************************************************************************
 * @file           : main.c
 * @author         : piro.tex
 * @brief          : Program to control fan speed
 *******************************************************************************
*/


#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>
 
#define PIN               16
#define RANGE             100
 
#define PWM_VALUE1         35
#define PWM_VALUE2         50
#define PWM_VALUE3         75
#define PWM_VALUE4         100
 
#define TEMPERATURE_1      45
#define TEMPERATURE_2      50
#define TEMPERATURE_3      60
#define TEMPERATURE_4      70
 
using namespace std;
 
static int getTemperature() {
  static fstream myfile;
  int temperature = 0;
  myfile.open("/sys/devices/virtual/thermal/thermal_zone0/temp", ios_base::in);
  myfile >> temperature;
  myfile.close();
  return (temperature + 30000) / 1000; // Adjustment
}
 
int main() {
  int temperature;
  bool pwmStopped = true;
 
  try {
    if (wiringPiSetup() == 0) {
      while (1) {
        temperature = getTemperature();
 
        if (temperature > TEMPERATURE_4) {
          if (pwmStopped) {
            softPwmCreate(PIN, ((PWM_VALUE4 * RANGE) / 100), RANGE);
            pwmStopped = false;
          } else {
            softPwmWrite(PIN, ((PWM_VALUE4 * RANGE) / 100));
          }
        } else if (temperature > TEMPERATURE_3) {
          if (pwmStopped) {
            softPwmCreate(PIN, ((PWM_VALUE3 * RANGE) / 100), RANGE);
            pwmStopped = false;
          } else {
            softPwmWrite(PIN, ((PWM_VALUE3 * RANGE) / 100));
          }
        } else if (temperature > TEMPERATURE_2) {
          if (pwmStopped) {
            softPwmCreate(PIN, ((PWM_VALUE2 * RANGE) / 100), RANGE);
            pwmStopped = false;
          } else {
            softPwmWrite(PIN, ((PWM_VALUE2 * RANGE) / 100));
          }
        } else if (temperature > TEMPERATURE_1) {
          if (pwmStopped) {
            softPwmCreate(PIN, ((PWM_VALUE1 * RANGE) / 100), RANGE);
            pwmStopped = false;
          } else {
            softPwmWrite(PIN, ((PWM_VALUE1 * RANGE) / 100));
          }
        } else {
          softPwmStop(PIN);
          pwmStopped = true;
        }
 
        usleep(1000 * 1000);
      }
    }
  } catch (exception& e) {
    cerr << e.what() << endl;
  }
  return 0;
}