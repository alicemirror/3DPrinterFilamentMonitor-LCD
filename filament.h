/* 
 *  Filament material parameters
 *  
 *  PLA
 *  Density: 1.25 g/cm^3
 *  Volume: 0.80 cm^3/g or 800 cm^3/kg
 *  1.75 mm filament length for 1 kg spool: ~ 330 meters
 *  3.00 mm filament length for 1 kg spool: ~ 110 meters
 *  
 *  ABS
 *  Density: 1.04 g/cm^3
 *  Volume: 0.96 cm^3/g or 960 cm^3/kg
 *  1.75 mm filament length for 1 kg spool: ~ 400 meters
 *  3.00 mm filament length for 1 kg spool: ~ 130 meters
 *  
 *  Based on these average information we define the following:
 */

// Switch matrial ID
#define PLA 0
#define ABS 1

// Material identifier
#define PLA175 0
#define PLA300 1
#define ABS175 2
#define ABS300 3

// Roll tare (gr)
#define ROLL1KG_TARE 240.0
#define ROLL2KG_TARE 250.0 // Not real!!! Should be known

// Roll weight ID
#define ROLL1KG 0
#define ROLL2KG 1

#define SYS_READY "Ready"       // System ready
#define SYS_PRINTING "Run"      // Filament in use
#define SYS_LOAD "Load"         // Roll loaded

// Status codes
#define STAT_NONE 0
#define STAT_READY 1
#define STAT_PRINTING 2
#define STAT_LOAD 3

#define DIAMETER175 "1.75 mm"
#define DIAMETER300 "3.00 mm"

#define UNITS_GR "gr"
#define UNITS_CM "cm"
#define UNITS_MT "m"
#define UNITS_KG "Kg"
#define FILAMENT_ROLL "Roll"

#define PLA_MAT "PLA"
#define ABS_MAT "ABS"

// Length in cm for 1 gr 1.75 mm PLA filament
#define PLA175_1GR_CM 33.0
// Weight in gr for 1 cm 1.75 mm PLA filament
#define PLA175_1CM_GR 0.03

// Length in cm for 1 gr 3.00 mm PLA filament
#define PLA300_1GR_CM 11.0
// Weight in gr for 1 cm 3.00 mm PLA filament
#define PLA300_1CM_GR 0.09
 
// Length in cm for 1 gr 1.75 mm ABS filament
#define ABS175_1GR_CM 40.0
// Weight in gr for 1 cm 1.75 mm ABS filament
#define ABS175_1CM_GR 0.025

// Length in cm for 1 gr 3.00 mm ABS filament
#define ABS300_1GR_CM 13.0
// Weight in gr for 1 cm 3.00 mm ABS filament
#define ABS175_1CM_GR 0.076

// 1Kg Roll spool average weight in gr (tare)
#define ROLL_WEIGHT 240

// This value is calculated sperimentally and is the zero weight for the
// used 3D printed model
#define SCALE_CALIBRATION 428.0

// Minimum weight difference to display an update in grams
#define SCALE_RESOLUTION 0.15

// Number of samples read every loop cycle. Multiple samples reading gives
// more stability to the measure
#define SCALE_SAMPLES 10

// Minimum number of grams variation between two reading too high
// to be considered weight change (force tension from the Extruder)
#define MIN_EXTRUDER_TENSION 50

// Filament material input pin
#define MATERIAL_PIN 7
#define DIAMETER_PIN 9
#define WEIGHT_PIN 11

// Control buttons pins
#define RESTART_PIN A0
#define SETZERO_PIN 12

// LED pin on when reading
#define READING_PIN 13


