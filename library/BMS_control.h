#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  //...
} VoltageSelection;

typedef enum {
  //...
} CurrentSelection;

typedef enum {
  //...
} TemperatureSelection;

typedef enum {
  //...
} DCtoDCSelection;

double Get_Voltage(VoltageSelection v_sel);
double Get_Current(CurrentSelection I_sel);
double Get_Temperature(TemperatureSelection T_sel);
bool Enable(DCtoDCSelection DC_sel);
