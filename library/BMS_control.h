#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  PA0,
  PA4,
  PA5,
  PA6,
  PF4,
  PF5,
  PF7
} VoltageSelection;

typedef enum {
  PF9,
  PF10
} CurrentSelection;

typedef enum {
  PB1,
  PC0,
  PC2,
  PC3
} TemperatureSelection;

typedef enum {
  PE2,
  PD11,
  PD12,
  PD13,
  PA0
} DCtoDCSelection;

double Get_Voltage(VoltageSelection v_sel);
double Get_Current(CurrentSelection I_sel);
double Get_Temperature(TemperatureSelection T_sel);
bool Enable(DCtoDCSelection DC_sel);
