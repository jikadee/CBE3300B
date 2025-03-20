// **TDS Sensor Setup**
#define TdsSensorPin A1

#define VREF 5.0      // Analog reference voltage (5V)
#define SCOUNT 30     // Number of samples for averaging
int analogBuffer[SCOUNT];  
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

// **Pump Setup**
#define PUMP_PIN 9  // Pump control pin

// **TDS Control Limits**
#define TDS_THRESHOLD 50   // TDS threshold for pump control

void setup() {
    Serial.begin(115200);
    pinMode(TdsSensorPin, INPUT);  // TDS sensor pin
    pinMode(PUMP_PIN, OUTPUT);    // Pump control pin (pin 9)
}

void loop() {
    readTDS();  // Read TDS sensor
    controlPump();  // Control pump based on TDS value
}

// **Reads TDS value from the sensor**
void readTDS() {
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U) { 
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
        analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
    }

    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U) {
        printTimepoint = millis();
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
        float compensationVoltage = averageVoltage / compensationCoefficient;
        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage 
                   - 255.86 * compensationVoltage * compensationVoltage 
                   + 857.39 * compensationVoltage) * 0.5;

    }

        Serial.print("TDS Value: ");
        Serial.print(tdsValue);
        Serial.println(" ppm");
}

// **Controls the pump based on TDS**
void controlPump() {
    if (tdsValue > TDS_THRESHOLD) {
        // Turn pump ON
        digitalWrite(PUMP_PIN, HIGH);
        Serial.println("Pump ON");
    } else {
        // Turn pump OFF
        digitalWrite(PUMP_PIN, LOW);
        Serial.println("Pump OFF");
    }
}

// **Median filter to stabilize TDS readings**
int getMedianNum(int bArray[], int iFilterLen) {
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];

    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++) {
        for (i = 0; i < iFilterLen - j - 1; i++) {
            if (bTab[i] > bTab[i + 1]) {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }

    if ((iFilterLen & 1) > 0) return bTab[(iFilterLen - 1) / 2];
    else return (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}
