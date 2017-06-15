/*

  3D Printer FIlament Monitor - Version with buttons and LCD display

 Arduino pins
 HX711 CLK
 DAT
 VCC
 GND
 
 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

 Licensed under GNU GPL 4.0
 Author:  Enrico Miglino. (c) 2017
          Balearic Dynamics SL balearicdynamic@gmail.com
 Updated sources on GitHUB: https://github.com/alicemirror/3DPrinterFilamentMonitor-LCD/

*/

#include <HX711.h>
#include <ShiftLCD.h>
#include <Streaming.h>
#include "filament.h"

#define DOUT 5  // load sensor data pin
#define CLK 6   // load sensor clock pin

// Load sensor library initialisation
HX711 scale(DOUT, CLK);

// LCD library initialisation
ShiftLCD lcd(2, 3, 4);

// filament diameter (descriptive)
String diameter;
// material type (descriptive)
String material;
// roll weight (descriptive)
String weight;
// system status
String stat;
// Status ID
int statID;
// grams for 1 cm material
float gr1cm;
// centimeters for 1 gr material
float length1gr;
// filament weight
float rollWeight;
// roll tare
float rollTare;
// Last read value from the cell
float lastRead;
// Previous read value from the cell
float prevRead;
// Initial read weight from last reset
float initialWeight;

// Units display flag. Decide if consume is in grams or cm
float filamentUnits;

#undef DEBUG

// ==============================================
// Initialisation
// ==============================================
void setup() {
#ifdef DEBUG
  Serial.begin(38400);
#endif
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  // Print a message to the LCD.
  lcd.print("BalearicDynamics");
  lcd.setCursor(0, 1);
  lcd.print("Filament Monitor");
  delay(3000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Ver. 1.1.14");
  lcd.setCursor(0,1);
  lcd.print("Calibrating...");
  delay(1000);

  // Initialise the scale with the model calibration factor then set the initial weight to 0
  scale.set_scale(SCALE_CALIBRATION);
  scale.tare();

  // Set the dip switch pins
  pinMode(MATERIAL_PIN, INPUT);   // Dip switch material select
  pinMode(DIAMETER_PIN, INPUT);   // Dip switch filament diameter select
  pinMode(WEIGHT_PIN, INPUT);     // Dip switch filament weight select
  pinMode(READING_PIN, OUTPUT);   // LED reading signal
  pinMode(SETZERO_PIN, INPUT);    // Push button state change
  pinMode(CHANGE_UNIT_PIN, INPUT);   // Push button restart material

  // Initialised the default values for the default filament type
  setDefaults();

  lcd.clear();
  showInfo();
  delay(2000);
}

// ==============================================
// Main loop
// ==============================================

/* 
 * The main loop role is execturing the service functions; display update, 
 * calculations, button checking
 * The scale reading is done at a specific frequence and is interrupt-driven
 */
void loop() {
  // Get the last reading
  prevRead = lastRead;
  lastRead = readScale();

  digitalWrite(READING_PIN, HIGH); // LED Enable

  // Check the current status of the system
  switch(statID) {
    case STAT_READY:
      // Set the initial weight to calculate the consumed material during a session
      initialWeight = lastRead;
      prevRead = lastRead;
      showLoad();
      break;
    case STAT_LOAD:
      showStat();
      break;
    case STAT_PRINTING:
      // Avoid fluctuations due the extruder tension
      if( (lastRead-prevRead) >= MIN_EXTRUDER_TENSION) {
        // Restore the previous reading
        lastRead = prevRead;
      }
      showStat();
      break;  
    default:
        showInfo();
        break;
    } // switch

  // Manage the status change button
    if(digitalRead(SETZERO_PIN)) {
      delay(100); // Barbarian debouncer
        if(statID == STAT_NONE) {
          statID = STAT_READY;
          stat = SYS_READY;
          return;
        }
        if(statID == STAT_READY) {
          stat = SYS_LOAD;
          statID = STAT_LOAD;
          lcd.clear();
          return;
        } 
        if(statID == STAT_LOAD) {
          // Change from load to running mode
          stat = SYS_PRINTING;
          statID = STAT_PRINTING;
          lcd.clear();
          return;
        } 
        if(statID == STAT_PRINTING) {
          // Change from load to running mode
          stat = SYS_LOAD;
          statID = STAT_LOAD;
          // Set the initial weight to calculate the consumed material during a session
          initialWeight = lastRead;
          lcd.clear();
          return;
        }
  }

  // Manage the partial consumption button. Only when STAT_PRINTING
  if(digitalRead(CHANGE_UNIT_PIN)) {
    delay(250); // Barbarian debouncer
#ifdef DEBUG
  Serial << "CHANGE_UNIT_PIN = " << stat << " filamentUnits = " << filamentUnits;
  Serial.println();
#endif
    if(filamentUnits == _GR) {
      filamentUnits = _CM;
    }
    else {
      filamentUnits = _GR;
    }
  }

  digitalWrite(READING_PIN, LOW);
}

// ==============================================
//  SCALE METHODS
// ==============================================

/* 
 * Set the gloabl values depending on the material and filament size parameters
 * read from the dip switch
 * 
 * \param filament The filament ID
 * \param wID The rolld weight ID
 */
void setDefaults() {
  int wID;  // Filament weight ID
  int diameterID; // Roll diameter
  int materialID; // Roll material
  int filament; // Filament type

  stat = SYS_STARTED;
  statID = STAT_NONE;
  lastRead = 0;
  prevRead = 0;
  filamentUnits = _GR;  // default filament units

  diameterID = digitalRead(DIAMETER_PIN);
  materialID = digitalRead(MATERIAL_PIN);
  wID = digitalRead(WEIGHT_PIN);

  // Calculate the material + diameter ID
  if(materialID == PLA) {
    filament = PLA175 + diameterID; 
  }
  else if(materialID == ABS) {
    filament = ABS175 + diameterID;
  }

  // Set the weight filament and tare for the supported
  // roll types
  switch(wID) {
    case ROLL1KG:
      rollWeight = 1000.0;
      rollTare = ROLL1KG_TARE;
      weight = "1";
      break;
    case ROLL2KG:
      rollWeight = 2000.0;
      rollTare = ROLL2KG_TARE;
      weight = "2";
      break;
  }

  // Set the parameters depending on the filament
  // characteristics.
  switch(filament) {
    case PLA175:
      diameter = DIAMETER175;
      material = PLA_MAT;
      gr1cm = PLA175_1CM_GR;
      length1gr = PLA175_1GR_CM;
      break;
    case PLA300:
      diameter = DIAMETER300;
      material = PLA_MAT;
      gr1cm = PLA300_1CM_GR;
      length1gr = PLA300_1GR_CM;
      break;
    case ABS175:
      diameter = DIAMETER175;
      material = ABS_MAT;
      gr1cm = ABS175_1CM_GR;
      length1gr = ABS175_1GR_CM;
      break;
    case ABS300:
      diameter = DIAMETER300;
      material = ABS_MAT;
      gr1cm = ABS175_1CM_GR;
      length1gr = ABS300_1GR_CM;
      break;
  }
}

/*
 * Exectues a scale series of readings without the plastic spool
 * (and any other extra weight that is not part of the measure)
 */
float readScale(void) {

  return (scale.get_units(SCALE_SAMPLES) * -1) - rollTare;
}

// ==============================================
//  LCD METHODS
// ==============================================

/* 
 * Show the filament information preset
 */
void showInfo() {
  
  lcd.setCursor(0,0);
  lcd << material << " " << diameter << " " << weight << " " << UNITS_KG << "  ";
  lcd.setCursor(0,1);
  lcd << stat << "    ";
#ifdef DEBUG
  Serial << "showInfo()stat = " << stat;
  Serial.println();
#endif
}

/* 
 * Show the filament information after the roll has been loaded
 */
void showLoad() {
  
  lcd.setCursor(0,0);
  lcd << material << " " << diameter << " " << weight << " " << UNITS_KG;
  lcd.setCursor(0,1);
  lcd << stat << " " << calcRemainingPerc(lastRead) << "%    ";
#ifdef DEBUG
  Serial << "showLoad() calcRemainingPerc(lastRead) = " << calcRemainingPerc(lastRead);
  Serial.println();
#endif
}

/* 
 * Show the filament status while the printing is running
 */
void showStat() {
  float consumedGrams;
  consumedGrams = calcConsumedGrams();
  if(consumedGrams < 0)
    consumedGrams = 0;
  
  lcd.setCursor(0,0);
  lcd << calcGgramsToCentimeters(lastRead)/100 << " " << UNITS_MT << " " << calcRemainingPerc(lastRead) << "%";
  lcd.setCursor(0,1);

  if(filamentUnits == _GR)
    lcd << stat << " " << consumedGrams << " " << UNITS_GR << "  ";
  else
    lcd << stat << " " << calcGgramsToCentimeters(consumedGrams) << " " << UNITS_CM << "  ";
    
#ifdef DEBUG
  Serial << "showStat() consumedGrams = " << consumedGrams;
  Serial.println();
#endif
}

// ==============================================
//  CALC METHODS
// ==============================================

/** 
 *  Caclulate the centimeters for the corresponding weight
 *  
 *  The applied formula: (w - tare) / gr1cm
 *  
 *  \param w weight in grams
 *  \return the length in centimeters
*/
float calcGgramsToCentimeters(float w) {
 return w / gr1cm;
}

/*
 * Calculate the remaining weight percentage of filament
 * 
 * The applied forumal: (w - tare) * 100 / roll weight
 */
float calcRemainingPerc(float w) {
  return w * 100 / rollWeight;
 }

 /*
  * Calculate the consumed material after the roll loading in grams
  * 
  * the applied formula: initialWeight - lastRead - tare
  */
  float calcConsumedGrams() {
    return initialWeight - lastRead;
  }

 /*
  * Calculate the consumed material after the roll loading in centimeters
  */
  float calcConsumedCentimeters() {
    return calcGgramsToCentimeters(calcConsumedGrams());
  }


