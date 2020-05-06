#include <ArduinoSTL.h>      // https://github.com/mike-matera/ArduinoSTL
#include <Cylinder.h>        // https://github.com/chischte/cylinder-library
#include <Debounce.h>        // https://github.com/chischte/debounce-library
#include <Insomnia.h>        // https://github.com/chischte/insomnia-delay-library
#include <EEPROM_Counter.h>  // https://github.com/chischte/eeprom-counter-library
#include <EEPROM_Logger.h>   // https://github.com/chischte/eeprom-logger-library.git
#include <CycleStep.h>       //
#include <StateController.h> // https://github.com/chischte/state-controller-library.git

//******************************************************************************
// DEFINE NAMES AND SET UP VARIABLES FOR THE CYCLE COUNTER:
//******************************************************************************
enum counter
{
  longtimeCounter,  //
  shorttimeCounter, //
  coolingTime,      // [s]
  endOfCounterEnum
};

int counterNoOfValues = endOfCounterEnum;

//******************************************************************************
// DEFINE NAMES AND SET UP VARIABLES FOR THE ERROR LOGGER:
//******************************************************************************
enum logger
{
  emptyLog, // marks empty logs
  toolResetError,
  shortTimeoutError,
  longTimeoutError,
  shutDownError,
  magazineEmpty,
  manualOn,
  manualOff
};

String error_code[] = { //
    "n.a.",             //
    "STEUERUNG EIN",    //
    "AUTO RESET",       //
    "AUTO PAUSE",       //
    "AUTO STOP",        //
    "BAND LEER",        //
    "MANUELL START",    //
    "MANUELL STOP"};    //

int logger_no_of_logs = 50;

//******************************************************************************
// DECLARATION OF VARIABLES
//******************************************************************************
bool strapDetected;
bool errorBlinkState = false;
byte timeoutDetected = 0;
int cycleTimeInSeconds = 30; // estimated value for the timout timer

//******************************************************************************
// GENERATE INSTANCES OF CLASSES:
//******************************************************************************
Cylinder cylinder_schlitten(5);
Cylinder cylinder_bandklemme(6);

Insomnia errorBlinkTimer;
unsigned long blinkDelay = 600;
Insomnia resetTimeout; // reset rig after 40 seconds inactivity
Insomnia resetDelay;
Insomnia coolingDelay;
Insomnia nexResetButtonTimeout;

StateController state_controller;
EEPROM_Counter eeprom_counter;
//******************************************************************************
// WRITE CLASSES FOR THE MAIN CYCLE STEPS
//******************************************************************************
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Step_wippenhebel : public Cycle_step
{
public:
  String get_display_string() { return "WIPPENHEBEL"; }
  void do_stuff()
  {
    cylinder_schlitten.stroke(1500, 1000);
    eeprom_counter.setup(0, 1023, 20);
    if (cylinder_bandklemme.stroke_completed())
      Serial.println("Class I bytes ya tooth");
    set_solved();
  }

private:
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Step_band_klemmen : public Cycle_step
{
public:
  String get_display_string() { return "KLEMMEN"; }
  void do_stuff()
  {
    Serial.println("Class II bytes me teeth");
    set_solved();
  }

private:
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//******************************************************************************
// CREATE VECTOR CONTAINER FOR THE CYCLE STEPS OBJECTS
//******************************************************************************
int Cycle_step::objectCount = 0; // enable object counting
std::vector<Cycle_step *> cycle_steps;
//*****************************************************************************

void setup()
{
  //------------------------------------------------
  // PUSH THE CYCLE STEPS INTO THE VECTOR CONTAINER:
  // PUSH SEQUENCE = CYCLE SEQUENCE!
  //------------------------------------------------
  cycle_steps.push_back(new Step_wippenhebel);
  cycle_steps.push_back(new Step_band_klemmen);
  //------------------------------------------------
  // Get number of cycles from parent class:
  int no_of_cycle_steps = Cycle_step::objectCount;
  state_controller.setNumberOfSteps(no_of_cycle_steps);
  //------------------------------------------------
  Serial.begin(115200);
  Serial.println("EXIT SETUP");
}
void loop()
{

  // TODO:
  // Implement step/auto logic from bxt standard rig
  // If smart, add timeout possibility to abstract cycle step class

  // GET AND RUN CURRENT STEP:
  int currentStep = state_controller.currentCycleStep();
  std::cout << "STEP NUMBER: " << currentStep << "\n";
  cycle_steps[currentStep]->do_stuff();

  // IF STEP IS COMPLETED SWITCH TO NEXT STEP:
  if (cycle_steps[currentStep]->is_completed())
  {
    Serial.println("STEP COMPLETED");
    state_controller.switchToNextStep();
  }
  delay(1000);
}