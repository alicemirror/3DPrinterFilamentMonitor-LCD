/*

  3D Printer FIlament Monitor - Version with buttons and LCD display

 Arduino pins
 HX711 CLK
 DAT
 VCC
 GND
 
 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.

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
// Triggering weight during initialisation
float weightTrigger;

// ==============================================
//  SCALE METHODS
// ==============================================

/* 
 * Set the gloabl values depending on the material and filament size 
 * 
 * \param filament The filament ID
 * \param wID The rolld weight ID
 */
void setDefaults() {
  int wID;  // Filament weight ID
  int diameterID; // Roll diameter
  int materialID; // Roll material
  int filament; // Filament type

  stat = SYS_READY;
  statID = STAT_NONE;
  lastRead = 0;
  prevRead = 0;

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

  weightTrigger = rollTare;

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
 * Exectues a scale series of readings 
 */
float readScale(void) {

  return (scale.get_units(SCALE_SAMPLES) * -1) - rollTare;
}

// ==============================================
//  LCD METHODS
// ==============================================

/* 
 * Show the filament information 
 */
void showInfo() {
  
  lcd.setCursor(0,0);
  lcd << material << " " << diameter << " " << weight << " " << UNITS_KG;
  lcd.setCursor(0,1);
  lcd << stat;
}

/* 
 * Show the filament information 
 */
void showLoad() {
  
  lcd.setCursor(0,0);
  lcd << material << " " << diameter << " " << weight << " " << UNITS_KG;
  lcd.setCursor(0,1);
  lcd << stat << " " << calcRemainingPerc(lastRead) << "%";
}

/* 
 * Show the filament status 
 */
void showStat() {
  float consumedGrams;
  
  lcd.setCursor(0,0);
  lcd << calcGgramsToCentimeters(lastRead)/100 << " " << UNITS_MT << " " << calcRemainingPerc(lastRead) << "%";
  lcd.setCursor(0,1);

  consumedGrams = calcConsumedGrams();
  if(consumedGrams > 0) {
    lcd << stat << " " << consumedGrams << " " << UNITS_GR;
  }
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
  lcd.print("Ver. 1.1.11");
  lcd.setCursor(0,1);
  lcd.print("Calibrating...");
  delay(1000);

  // Initialise the scale with the model calibration factor then set the initial weight to 0
  scale.set_scale(SCALE_CALIBRATION);
  scale.tare();

  // Set the dip switch pins
  pinMode(MATERIAL_PIN, INPUT);
  pinMode(DIAMETER_PIN, INPUT);
  pinMode(WEIGHT_PIN, INPUT);
  pinMode(READING_PIN, OUTPUT);
  pinMode(SETZERO_PIN, INPUT);
  pinMode(RESTART_PIN, OUTPUT);

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
 * 
 * Note: the weightTrigger is used to reduce the number of inital valid readings
 * then when the system is in PRINTING (run) mode it is set to zero to accept
 * every value and keep the display updated more frequently.
 */
void loop() {
  // Get the last reading
  prevRead = lastRead;
  lastRead = readScale();

//  // Avoid fluctuations due the extruder tension
//  if( (lastRead-prevRead) >= MIN_EXTRUDER_TENSION) {
//    // Restore the previous reading
//    lastRead = prevRead;
//  }

#ifdef DEBUG
  Serial << "lastRead = " << lastRead << " prevRead = " << prevRead << " SCALE_RESOLUTION = " << SCALE_RESOLUTION << " prev-last = " << prevRead - lastRead;
  Serial.println();
#endif

  digitalWrite(READING_PIN, HIGH);

    switch(statID) {
      case STAT_READY:
        // Set the initial weight to calculate the consumed material during a session
        initialWeight = lastRead;
        prevRead = lastRead;
        showLoad();
        weightTrigger = 0;
        break;
      case STAT_LOAD:
        showStat();
        break;
      case STAT_PRINTING:
        int deltaRead;
        deltaRead = prevRead - lastRead;
        showStat();
        break;  
      default:
          showInfo();
          break;
      } // switch

      if(digitalRead(SETZERO_PIN)) {
        delay(100); // Barbarian debouncer
#ifdef DEBUG
  Serial << "SETZERO_PIN pressed " << digitalRead(SETZERO_PIN);
  Serial.println();
#endif
        if(statID == STAT_NONE) {
          statID = STAT_READY;
          stat = SYS_READY;
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
  } // BUTTON
#ifdef DEBUG
  Serial << "SETZERO_PIN true " << lastRead << " stat = " << stat << " statID = " << statID;
  Serial.println();
#endif

  digitalWrite(READING_PIN, LOW);
}



