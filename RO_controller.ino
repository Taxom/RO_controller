/*
-------------------------------
RO-filter_control_v0.6
@Pivovarov Maxim
25/03/2024
Arduino_mini_pro
-------------------------------
*/

const int INPUT_PRESSURE_SENSOR_PIN = 12; // LOW if water is available 
const int OUTPUT_PRESSURE_SENSOR_PIN = 11; // HIGH if the tank is full
const int FLUSH_SET1_PIN = A0;
const int FLUSH_SET2_PIN = A1;
const int BUTTON_FLUSH = A2; // active LOW
// For all outputs, active is HIGH
const int INPUT_VALVE_PIN = 2; 
const int FLUSH_VALVE_PIN = 4;
const int DRAIN_VALVE_PIN = 9;
const int PUMP_PIN = 3;
const int FILTERING_LED_PIN = 6;
const int FLUSH_LED_PIN = 7;
const int WATER_LED_PIN = 5;


const unsigned long FLUSH_PERIOD_6_HOURS = 6UL * 60UL * 60UL * 1000UL;   // 6 hours in milliseconds
const unsigned long FLUSH_PERIOD_12_HOURS = 12UL * 60UL * 60UL * 1000UL; // 12 hours in milliseconds
const unsigned long FLUSH_PERIOD_24_HOURS = 24UL * 60UL * 60UL * 1000UL; // 24 hours in milliseconds


const int FLUSH_DELAY = 15*1000; // duration of periodic flushing 
const int PREFLUSH_DELAY = 10*1000; // duration of pre-production flushing
const int POSTFLUSH_DELAY = 10*1000; // duration of post-production flushing
const int POSTDREIN_DELAY = 5*1000; // duration of permeate pressure release
const int PREDRAINE_DELAY = 30*1000; // duration of initial permeate release
const int PRESSURE_RELEASE_DELAY = 3*1000; // duration of pressure release after pump shutdown
const int HYSTERESIS = 10*1000; // duration of operation after tank full sensor triggers
const int BOOT_FLASH = 5; // boot delay, 1 flash 0.25+0.25 seconds

unsigned long previousFlash = 0;
unsigned long lastFlushTime = 0;
unsigned long flushPeriod = 0;
// bool waterInput = 0 // Instead of a variable, use the WATER_LED_PIN port; if water is available, then HIGH (on)


// Function declarations
void setFlushPeriod();
void startWaterPreparation();
void flushMembrane();
bool checkInputPressure();
void delayWithoutBlocking(unsigned long ms);


void setup() {

  // Pressure sensors
  pinMode(INPUT_PRESSURE_SENSOR_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_PRESSURE_SENSOR_PIN, INPUT_PULLUP);
  // Jumpers and flush button
  pinMode(FLUSH_SET1_PIN, INPUT_PULLUP);
  pinMode(FLUSH_SET2_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_FLUSH, INPUT_PULLUP);
  // Valves and pump
  pinMode(INPUT_VALVE_PIN, OUTPUT);
  pinMode(FLUSH_VALVE_PIN, OUTPUT);
  pinMode(DRAIN_VALVE_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  // Status indicators
  pinMode(FILTERING_LED_PIN, OUTPUT);
  pinMode(FLUSH_LED_PIN, OUTPUT);
  pinMode(WATER_LED_PIN, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // Set all outputs to LOW
  digitalWrite(INPUT_VALVE_PIN, LOW);
  digitalWrite(FLUSH_VALVE_PIN, LOW);
  digitalWrite(DRAIN_VALVE_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(FILTERING_LED_PIN, LOW);
  digitalWrite(FLUSH_LED_PIN, LOW);
  digitalWrite(WATER_LED_PIN, LOW);
  
  for (int i = 0; i<BOOT_FLASH; i++){
    digitalWrite (LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite (LED_BUILTIN, LOW);
    delay(250);
  }
}

void loop() {
  // Check jumper settings
  setFlushPeriod(); 


  // Sit here and blink LED_BUILTIN while there is no water
  while (digitalRead(INPUT_PRESSURE_SENSOR_PIN)){
    digitalWrite(WATER_LED_PIN, LOW);
    unsigned long currentMillis = millis();

    if (currentMillis - previousFlash >= 1000) { 
        previousFlash = currentMillis;
        int ledState = digitalRead(LED_BUILTIN);
        digitalWrite(LED_BUILTIN, !ledState);
    }

  }


  // On first start or if water appears
  if (!digitalRead(WATER_LED_PIN) && (!digitalRead(INPUT_PRESSURE_SENSOR_PIN))){
    digitalWrite(WATER_LED_PIN, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
	  flushMembrane(); // Start flushing
  }  


  // If it's time for flushing and water is available
  if (flushPeriod && (millis() - lastFlushTime >= flushPeriod) && digitalRead(WATER_LED_PIN) && (!digitalRead(INPUT_PRESSURE_SENSOR_PIN))) { 
    flushMembrane(); // Start flushing
  }
  

  // Flush button pressed
  if (!digitalRead(BUTTON_FLUSH) && digitalRead(WATER_LED_PIN) && !digitalRead(INPUT_PRESSURE_SENSOR_PIN)){
	  flushMembrane(); // Start flushing
  }
  

  // If the tank is empty and water is available  
  if (digitalRead(WATER_LED_PIN) && (!digitalRead(OUTPUT_PRESSURE_SENSOR_PIN))){ 
	  startWaterPreparation(); // Start filtration
  }

}


// Check jumpers and set flushing time
void setFlushPeriod() {   
  bool set1 = !digitalRead(FLUSH_SET1_PIN);
  bool set2 = !digitalRead(FLUSH_SET2_PIN);

  if (set1 && !set2) {
    flushPeriod = FLUSH_PERIOD_6_HOURS;
  } else if (!set1 && set2) {
    flushPeriod = FLUSH_PERIOD_12_HOURS;
  } else if (set1 && set2) {
    flushPeriod = FLUSH_PERIOD_24_HOURS;
  } else {
    flushPeriod = 0; // No flushing if no jumpers are set
  }
}


// Filtration function
void startWaterPreparation() {    
	// Turn on the "filtration" indicator
	digitalWrite(FILTERING_LED_PIN, HIGH);
	// Open all valves and turn on the pump
  digitalWrite(INPUT_VALVE_PIN, HIGH);
  digitalWrite(FLUSH_VALVE_PIN, HIGH);
  digitalWrite(DRAIN_VALVE_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
	// Pre-flushing
  delayWithoutBlocking(PREFLUSH_DELAY);
	// After the first delay, close the flush valve
  digitalWrite(FLUSH_VALVE_PIN, LOW);
	// Let the initial permeate drain
  delayWithoutBlocking(PREDRAINE_DELAY);
	// Close the permeate drain valve
  digitalWrite(DRAIN_VALVE_PIN, LOW);
  // Wait for the tank to fill  
  while (!digitalRead(OUTPUT_PRESSURE_SENSOR_PIN) && (digitalRead(WATER_LED_PIN))) {  
	  // Check input pressure
    if (checkInputPressure()){        
      return;    
	  } 
  } 
  // Increase pressure for hysteresis
  delayWithoutBlocking(HYSTERESIS);  
	// Finish filtration with flushing if available
  if (digitalRead(WATER_LED_PIN)){
    digitalWrite(FLUSH_VALVE_PIN, HIGH);   
	  // Flush 
    delayWithoutBlocking(POSTFLUSH_DELAY);
	  // Turn off the pump and close the input valve	
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(INPUT_VALVE_PIN, LOW);
    // Wait for pressure release on the input
	  delayWithoutBlocking(PRESSURE_RELEASE_DELAY);
	  // Close the flush valve
    digitalWrite(FLUSH_VALVE_PIN, LOW);  
    // Reset periodic flush timer
    lastFlushTime = millis();	  
    // Turn off the "filtration" indicator and exit
    digitalWrite(FILTERING_LED_PIN, LOW);
  }
}


// Membrane flushing function
void flushMembrane() {
  // Turn on the "flushing" indicator
  digitalWrite(FLUSH_LED_PIN, HIGH);
  // Open all valves and start the pump
  digitalWrite(INPUT_VALVE_PIN, HIGH);
  digitalWrite(FLUSH_VALVE_PIN, HIGH);
  digitalWrite(DRAIN_VALVE_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
  // Flushing
  delayWithoutBlocking(FLUSH_DELAY);
  // Close valves and turn off the pump
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(INPUT_VALVE_PIN, LOW);
  digitalWrite(DRAIN_VALVE_PIN, LOW);
  // Wait for pressure release on the input 
  delayWithoutBlocking(PRESSURE_RELEASE_DELAY);
  // Close the flush valve
  digitalWrite(FLUSH_VALVE_PIN, LOW);  
  // Reset periodic flush timer
  lastFlushTime = millis();
  digitalWrite(FLUSH_LED_PIN, LOW);	
}

// Function to check pressure and protect the pump
bool checkInputPressure() {
  if (digitalRead(INPUT_PRESSURE_SENSOR_PIN)) {  // If water pressure is lost, start pump protection procedure
    digitalWrite(PUMP_PIN, LOW); 
    digitalWrite(INPUT_VALVE_PIN, LOW);
    digitalWrite(WATER_LED_PIN, LOW);
    digitalWrite(FILTERING_LED_PIN, LOW);
    digitalWrite(FLUSH_LED_PIN, LOW);
    // Open flush and drain valves to release pressure
    digitalWrite(FLUSH_VALVE_PIN, HIGH);
    digitalWrite(DRAIN_VALVE_PIN, HIGH);    
    // Wait for pressure release   
	  delay(PRESSURE_RELEASE_DELAY);
	  // Close flush and drain valves
    digitalWrite(FLUSH_VALVE_PIN, LOW);
    digitalWrite(DRAIN_VALVE_PIN, LOW);
	  return true;
  }  
  return false;
}

void delayWithoutBlocking(unsigned long ms) {  
  unsigned long start = millis();  
  while ((digitalRead(WATER_LED_PIN)) && (millis() - start < ms)) {
	  if (checkInputPressure()){ // Check pressure during waiting
    break;
	  }  
  }
}
