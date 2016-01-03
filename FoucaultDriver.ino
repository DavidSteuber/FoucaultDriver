/*
                        FOUCAULT DRIVER
                              BY
                         DAVID STEUBER
                           (C) 2015

  Use a photoresistor (light sensor) to control the magnet
  driving a Foucault Pendulum.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


Hardware connections:

  Photo resistor:
  
    Connect one side of the photoresistor to 5 Volts (5V).
    Connect the other side of the photoresistor to ANALOG pin 0.
    Connect a 10K resistor between ANALOG pin 0 and GND.

    This creates a voltage divider, with the photoresistor one
    of the two resistors. The output of the voltage divider
    (connected to A0) will vary with the light level.
    
  Yellow LED for light level indication:
  
    Connect the positive side (long leg) of the LED to
    digital pin 9. (To vary the brightness, this pin must
    support PWM, which is indicated by "~" or "PWM" on the
    Arduino itself.)

    Connect the negative side of the LED (short leg) to a
    330 Ohm resistor.

    Connect the other side of the resistor to GND.

  Green LED for detecting change in light level:

    Connect the positive side (long leg) of the LED to
    digital pin 12.

    Connect the negative side of the LED (short leg) to a
    330 Ohm resistor

*/


// As usual, we'll create constants to name the pins we're using.
// This will make it easier to follow the code below.

const int sensorPin = 0;         // For photoresistor
const int ledPin = 9;            // For light level indicator
const int indicator = 12;        // For light change (detecting the bob passing) indicator

void setup()
{
  // We'll set up the LED pin to be an output.
  // (We don't need to do anything special to use the analog input.)
  // Serial.begin(9600); // for debug output
  pinMode(ledPin, OUTPUT);
  pinMode(indicator, OUTPUT);
}


void loop()
{   
  int sensorValue = 0;
  
  if (sensorValue = readLightLevel())
  {
    /*
    if (sensorValue > 0)
      Serial.println("A");
    else
      Serial.println("B");
    */
      
    flashIndicator();  
  }
  delay(1); // let's just slow things down a bit for stability
}

int readLightLevel()
{
  const int numLevels = 16;
  static int averageLevels[numLevels];
  static int runningTotal = 0;
  static int averageLevelsIndex = 0;
  static int currentLevel = 0;
  int retVal = 0;
  int lightLevel = analogRead(sensorPin);

  autoTune(lightLevel);
  currentLevel++;
  runningTotal += lightLevel;

  if (currentLevel == numLevels)
  {
    currentLevel = 0;
    averageLevels[averageLevelsIndex] = runningTotal / numLevels;
    averageLevelsIndex++;
    runningTotal = 0;
    
    if (averageLevelsIndex == numLevels)
      averageLevelsIndex = 0;

    if (averageLevelsIndex > 0 && averageLevels[averageLevelsIndex] > (averageLevels[averageLevelsIndex - 1] + 8))
      retVal = 1; // bob is going
    if (averageLevelsIndex > 0 && (averageLevels[averageLevelsIndex] + 8) < averageLevels[averageLevelsIndex - 1])
      retVal = -1; // bob is comming
    if (averageLevels[numLevels -1] == 0)
      retVal = 0;  // averageLevels buffer is not full yet
/*
    Serial.print("average light level: ");
    for (int i = 0; i < numLevels; i++)
    {
      Serial.print(averageLevels[i]);
      Serial.print(", ");
    }
    Serial.println("");
*/
  }

  return retVal;
}

void autoTune(int lightLevel)
{
  static int high = 0, low = 1023;
  if (lightLevel < low)
  {
    low = lightLevel;
  }
  
  if (lightLevel > high)
  {
    high = lightLevel;
  }
    
  lightLevel = map(lightLevel, low+30, high-30, 0, 255);
  lightLevel = constrain(lightLevel, 0, 255);
  analogWrite(ledPin, lightLevel);
}

void flashIndicator()
{
  // flash the indicator LED when lightLevel is changing
  const unsigned long indicatorDuration = 50;
    
  digitalWrite(indicator, HIGH);
  delay(indicatorDuration);
  digitalWrite(indicator, LOW);
  delay(indicatorDuration);
}


