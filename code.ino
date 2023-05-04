#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#define SOL1_PIN PIN_PD4
#define SOL2_PIN PIN_PD5
#define SOL3_PIN PIN_PD6
#define MOTOR_PIN PIN_PD7

#define SINGLE_BUTT_PIN PIN_PC2
#define DOUBLE_BUTT_PIN PIN_PD3
#define MOCK_BUTT_PIN PIN_PD2
#define DISP_BUTT_PIN PIN_PC1

#define SINGLE_LED_PIN PIN_PB1
#define DOUBLE_LED_PIN PIN_PB2
#define MOCK_LED_PIN PIN_PB0

#define SCK_PIN PIN_PB5
#define LOAD_DATA_PIN PIN_PC0


//HX711 constructor:
HX711_ADC LoadCell(LOAD_DATA_PIN, SCK_PIN);

const int calVal_eepromAdress = 0;
unsigned long t = 0;


bool single_butt_pressed = false;
bool double_butt_pressed = false;
bool mock_butt_pressed = true;
bool disp_butt_pressed = false;

float sol1_mass = 0;
float sol2_mass = 0;
float sol3_mass = 0;



void setup() {
  pinMode(SOL1_PIN, OUTPUT); //Sets the pin as an output
  pinMode(SOL2_PIN, OUTPUT); //Sets the pin as an output
  pinMode(SOL3_PIN, OUTPUT); //Sets the pin as an output
  pinMode(MOTOR_PIN, OUTPUT); //Sets the pin as an output

  pinMode(SINGLE_BUTT_PIN, INPUT_PULLUP); //Sets the pin as an INPUT
  pinMode(DOUBLE_BUTT_PIN, INPUT_PULLUP); //Sets the pin as an INPUT
  pinMode(MOCK_BUTT_PIN, INPUT_PULLUP); //Sets the pin as an INPUT
  pinMode(DISP_BUTT_PIN, INPUT_PULLUP); //Sets the pin as an INPUT

  pinMode(SINGLE_LED_PIN, OUTPUT); //Sets the pin as an output
  pinMode(DOUBLE_LED_PIN, OUTPUT); //Sets the pin as an output
  pinMode(MOCK_LED_PIN, OUTPUT); //Sets the pin as an output

  digitalWrite(MOCK_LED_PIN, HIGH);
  digitalWrite(SINGLE_LED_PIN, LOW);
  digitalWrite(DOUBLE_LED_PIN, LOW);


  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = false; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

  LoadCell.setCalFactor(-971.8); // user set calibration value (float), initial value 1.0 may be used for this sketch
  Serial.println("Startup is complete");
  

  LoadCell.tare();

}

void loop() {

  if (digitalRead(SINGLE_BUTT_PIN) == LOW) {
    single_butt_pressed = true;
    double_butt_pressed = false;
    mock_butt_pressed = false;

    digitalWrite(SINGLE_LED_PIN, HIGH);
    digitalWrite(DOUBLE_LED_PIN, LOW);
    digitalWrite(MOCK_LED_PIN, LOW);

  }
  if (digitalRead(DOUBLE_BUTT_PIN) == LOW) {
    double_butt_pressed = true;
    single_butt_pressed = false;
    mock_butt_pressed = false;

    digitalWrite(DOUBLE_LED_PIN, HIGH);
    digitalWrite(SINGLE_LED_PIN, LOW);
    digitalWrite(MOCK_LED_PIN, LOW);

  }
  if (digitalRead(MOCK_BUTT_PIN) == LOW) {
    mock_butt_pressed = true;
    single_butt_pressed = false;
    double_butt_pressed = false;

    digitalWrite(MOCK_LED_PIN, HIGH);
    digitalWrite(SINGLE_LED_PIN, LOW);
    digitalWrite(DOUBLE_LED_PIN, LOW);

  }
  //if (digitalRead(DISP_BUTT_PIN) == LOW && readCupAdded() == true) {
  if (digitalRead(DISP_BUTT_PIN) == LOW) {

    if (single_butt_pressed && (!double_butt_pressed || !mock_butt_pressed)) {
      sol1_mass = 34.5;
      sol2_mass = 78.5;
      sol3_mass = 207;
    }
    if (double_butt_pressed && (!single_butt_pressed || !mock_butt_pressed)) {
      sol1_mass = 78;
      sol2_mass = 164.5;
      sol3_mass = 207;      
    }
    if (mock_butt_pressed && (!double_butt_pressed || !single_butt_pressed)) {
      sol1_mass = 0;
      sol2_mass = 0;
      sol3_mass = 207;
    }

    digitalWrite(MOCK_LED_PIN, HIGH);
    digitalWrite(SINGLE_LED_PIN, HIGH);
    digitalWrite(DOUBLE_LED_PIN, HIGH);


    LoadCell.tare();
    

    if (sol1_mass) {
      delay(2000);
      digitalWrite(SOL1_PIN, HIGH);
      readLoadCell(sol1_mass);
      digitalWrite(SOL1_PIN, LOW);
    }

    
    if (sol2_mass) {
      delay(2000);
      digitalWrite(SOL2_PIN, HIGH);
      readLoadCell(sol2_mass);
      digitalWrite(SOL2_PIN, LOW);
    }

    
    if (sol3_mass){
      delay(2000);
      digitalWrite(SOL3_PIN, HIGH);
      readLoadCell(sol3_mass);
      digitalWrite(SOL3_PIN, LOW);
    }

    delay(2000);
    digitalWrite(MOTOR_PIN, HIGH);
    delay(20000);
    digitalWrite(MOTOR_PIN, LOW);
    delay(2000);


      // reset states
    readLoadCellNegative(10);

    disp_butt_pressed = false;

    single_butt_pressed = false;
    double_butt_pressed = false;
    mock_butt_pressed = true;

    sol1_mass = 0;
    sol2_mass = 0;
    sol3_mass = 205;

    digitalWrite(MOCK_LED_PIN, HIGH);
    digitalWrite(SINGLE_LED_PIN, LOW);
    digitalWrite(DOUBLE_LED_PIN, LOW);
  }
  
}

void readLoadCell(float weight) {
  static boolean newDataReady = 0;
  float i;
  const int serialPrintInterval = 100; //increase value to slow down serial print activity

  while (1) {
      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;

      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          i = LoadCell.getData();
          Serial.print("Load_cell output val: ");
          Serial.println(i);
          newDataReady = 0;
          t = millis();
        }
      }

      if (i >= weight) {
        break;
      }

    }  

}

void readLoadCellNegative(float weight) {
  static boolean newDataReady = 0;
  float i = 0;
  const int serialPrintInterval = 100; //increase value to slow down serial print activity
  float sum = 0;

  while (1) {
      // check for new data/start next conversion:
      if (LoadCell.update()) newDataReady = true;

      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          i++;
          sum += LoadCell.getData();
          Serial.print("Load_cell output val: ");
          Serial.println(i);
          newDataReady = 0;
          t = millis();
        }
      }
      
      if (i == 10) {
        if (sum/i <= weight) {
          break;
        }
        i = 0;
        sum = 0;
      }

    }  

}
