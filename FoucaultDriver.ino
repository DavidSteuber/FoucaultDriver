/*
 *                         FOUCAULT DRIVER
 *                              BY
 *                         DAVID STEUBER
 *                           (C) 2015
 *
 *  Use a photoresistor (light sensor) to control the magnet
 *  driving a Foucault Pendulum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Hardware connections:
 *
 *  Photo resistor:
 * 
 *   Connect one side of the photoresistor to 5 Volts (5V).
 *   Connect the other side of the photoresistor to ANALOG pin 0.
 *   Connect a 10K resistor between ANALOG pin 0 and GND.
 *
 *    This creates a voltage divider, with the photoresistor one
 *    of the two resistors. The output of the voltage divider
 *    (connected to A0) will vary with the light level.
 *    
 *  Yellow LED for light level indication:
 *  
 *    Connect the positive side (long leg) of the LED to
 *    digital pin 9. (To vary the brightness, this pin must
 *    support PWM, which is indicated by "~" or "PWM" on the   
 *    Arduino itself.)
 *
 *    Connect the negative side of the LED (short leg) to a
 *    330 Ohm resistor.
 *
 *    Connect the other side of the resistor to GND.
 *
 *  Green LED for showing power to magnet:
 *
 *    Connect the positive side (long leg) of the LED to
 *    digital pin 7.
 *
 *    Connect the negative side of the LED (short leg) to a
 *    330 Ohm resistor
 *
 *    Connect the other side of the resistor to GND.
 */


// As usual, we'll create constants to name the pins we're using.
// This will make it easier to follow the code below.

const int sensorPin = 0;         // For photoresistor
const int ledPin = 9;            // For light level indicator
const int magnet = 7;            // For turning the magnet on and off. The green led indicates magnet state
const int SENSITIVITY = 34;     // Sensitivity to changing light level used by readLightLevel()
const int APPROACHING_BDC = 1;   // for the magnetControl() function
const int LEAVING_BDC = -1;      // for the magnetControl() function
const int MAX_DUTY = 200;        // The maximum time the magnet will be turned on by the magnetControl() function

void setup()
{
  //Serial.begin(9600); // for debug output
  pinMode(ledPin, OUTPUT); // Indicates ambient light level
  pinMode(magnet, OUTPUT); // Indicates magnet on
}


void loop()
{   
  int sensorValue = 0;
  
  if (sensorValue = readLightLevel())
  {
    
    if (sensorValue > 0)
      Serial.println("A");
    else
      Serial.println("B");
    
    magnetControl(sensorValue);  
  }
  // The following delay is actually needed for this to work
  delay(1);
}

/*
 *  This function reads the light level from the photoresistor and determins
 *  if the bob is approaching bottom dead center or leaving bottom dead center.
 *  Readings are averaged to smooth out noise as much as possible.
 *  
 *  Return value 0 means that the bob is not yet in view or the buffer for smoothing
 *  the data has not filled yet.
 *  
 *  Return value APPROACHING_BDC means the light level is reducing and therefor the
 *  pendulum bob is approaching bottom dead center.
 *  
 *  Return value LEAVING_BDC means the light level is increasing and therefor the
 *  pendulum bob is leaving bottom dead center.
 *  
 *  The global value SENSITIVITY is a threshold value to reduce false readings.
 */
 
int readLightLevel()
{
  const int numLevels = 16;
  static int averageLevels[numLevels];
  static int runningTotal = 0;
  static int averageLevelsIndex = 0;
  static int currentLevel = 0;
  int retVal = 0;
  int lightLevel = analogRead(sensorPin);

  autoTune(lightLevel); // Adjust the light level LED here
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

    if (averageLevels[numLevels -1] == 0)
      retVal = 0;  // averageLevels buffer is not full yet
    if (averageLevelsIndex > 0 && averageLevels[averageLevelsIndex] > (averageLevels[averageLevelsIndex - 1] + SENSITIVITY))
      retVal = APPROACHING_BDC; // bob is comming
    if (averageLevelsIndex > 0 && (averageLevels[averageLevelsIndex] + SENSITIVITY) < averageLevels[averageLevelsIndex - 1])
      retVal = LEAVING_BDC; // bob is going
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

/*
 * This function adjusts the light level indicator LED to show
 * relative ambient light level. The brighter the LED, the more
 * light there is.
 */
 
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

/*
 * This function is called when the light level is changing, indicating the
 * approach or departure of the pendulum bob. It keeps track of timing of
 * calls so that it can deal gracefully with spurious calls and tune
 * magnet duty cycles for an arbitrary length pendulum.
 */
 
void magnetControl(int toggle)
{
  /* 
   * keep track of states in case we get called more than once with the same
   * toggle value in a row. Missed states should not be a problem unless they
   * leave the magnet on. Setting a maximum duration should fix that.
   */
  static int toggleState = 0;
  static unsigned long approachTime = 0;
  static unsigned long departTime = 0;
  static unsigned long hangTime = 0;
  unsigned long maxDuration = 0;

  if (toggle == toggleState)
    return;

  toggleState = toggle;
  
  if (toggle == APPROACHING_BDC)
  {
    digitalWrite(magnet, HIGH);
    approachTime = millis();
    if (hangTime > 0)
    {
      maxDuration = min(hangTime / 2, MAX_DUTY);
      delay(maxDuration);
      digitalWrite(magnet, LOW);  
    }
  }
  if (toggle == LEAVING_BDC)
  {
    digitalWrite(magnet, LOW);
    departTime = millis();
    hangTime = departTime - approachTime;
  }
}


