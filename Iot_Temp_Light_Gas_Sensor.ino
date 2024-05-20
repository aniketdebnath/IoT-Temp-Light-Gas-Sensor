#include <DHT.h>

// Pin definitions
#define DHTPIN 2              // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11         // DHT 11 type
#define LDRPIN A0             // Analog pin connected to the LDR
#define BUZZER_PIN 8          // Digital pin connected to the buzzer
#define GREEN_LED_PIN 10      // Digital pin connected to the green LED
#define RED_LED_PIN 9         // Digital pin connected to the red LED
#define YELLOW_LED_PIN 11     // Digital pin connected to the yellow LED
#define GAS_SENSOR_PIN A1     // Analog pin connected to the gas sensor
#define MOTOR_PIN 3           // Digital pin connected to the motor

int GAS_THRESHOLD= 400;     // Threshold value for the gas sensor
int TEMP_THRESHOLD_HIGH= 20;// Temperature threshold
int LIGHT_LEVEL_DARK= 100;  // Light level threshold

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);  // Setup motor pin as output

  dht.begin();
  Serial.begin(9600);
}

// Global variables to track blinking state and timing
unsigned long previousMillis = 0;
const long ledBlinkInterval = 100;
const long buzzerInterval = 250;

// Function to blink any LED
void blinkLED(int pin) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= ledBlinkInterval) {
        previousMillis = currentMillis;
        digitalWrite(pin, !digitalRead(pin));
    }
}

// Function to beep the buzzer
void beepBuzzer() {
    static unsigned long lastBuzzerToggle = 0;
    static bool buzzerState = false;

    unsigned long currentMillis = millis();
    if (currentMillis - lastBuzzerToggle >= buzzerInterval) {
        lastBuzzerToggle = currentMillis;
        buzzerState = !buzzerState;
        digitalWrite(BUZZER_PIN, buzzerState);
    }
}

// Handle LEDs, buzzer, and motor based on sensor readings
void handleActuators(float temp, int lightLevel, int gasConcentration) {
    bool shouldBuzz = false;
    bool shouldRunMotor = false;

    if (temp > TEMP_THRESHOLD_HIGH) {
        blinkLED(RED_LED_PIN);
        shouldBuzz = true;
        shouldRunMotor = true;
    } else {
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
    }

    if (lightLevel < LIGHT_LEVEL_DARK) {
        digitalWrite(YELLOW_LED_PIN, HIGH);
        shouldBuzz = true;
        shouldRunMotor = true;
    } else {
        digitalWrite(YELLOW_LED_PIN, LOW);
    }

    if (gasConcentration >= GAS_THRESHOLD) {
        shouldBuzz = true;
        shouldRunMotor = true;
    }

    if (shouldBuzz) {
        beepBuzzer();
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }

    digitalWrite(MOTOR_PIN, shouldRunMotor ? HIGH : LOW);
}

void loop() {
    // Check for serial input to set thresholds dynamically
    if (Serial.available() > 0) {
        String serialInput = Serial.readStringUntil('\n'); // Read the serial input until a newline is found
        Serial.println("Received: " + serialInput);  // Echo for debugging

        if (serialInput.startsWith("SET_TEMP:")) {
            TEMP_THRESHOLD_HIGH = serialInput.substring(9).toInt();
            Serial.print("New temp threshold set to: ");
            Serial.println(TEMP_THRESHOLD_HIGH);
        } else if (serialInput.startsWith("SET_LIGHT:")) {
            LIGHT_LEVEL_DARK = serialInput.substring(10).toInt();
            Serial.print("New light level threshold set to: ");
            Serial.println(LIGHT_LEVEL_DARK);
        } else if (serialInput.startsWith("SET_GAS:")) {
            GAS_THRESHOLD = serialInput.substring(8).toInt();
            Serial.print("New gas threshold set to: ");
            Serial.println(GAS_THRESHOLD);
        }
    }

    // Read humidity and temperature
    float h = dht.readHumidity();
    float t = dht.readTemperature(); // Read temperature as Celsius
    int lightLevel = analogRead(LDRPIN); // Read light level
    lightLevel = map(lightLevel, 0, 1023, 0, 255); // Map light level to 0-255
    int gasConcentration = analogRead(GAS_SENSOR_PIN); // Read gas concentration

    // Handle actuators based on sensor values
    handleActuators(t, lightLevel, gasConcentration);

    // Print the results to the Serial Monitor in a comma-separated format
    Serial.print(h);
    Serial.print(",");
    Serial.print(t);
    Serial.print(",");
    Serial.print(lightLevel);
    Serial.print(",");
    Serial.print(gasConcentration);
    Serial.print(",");
    Serial.println(digitalRead(MOTOR_PIN));

    delay(1000); // Delay for a second before repeating the loop
}
