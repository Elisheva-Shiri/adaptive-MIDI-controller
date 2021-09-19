// Analog input of the horizontal joystick position
const int JoystickX = A0;
const int JoystickY = A1;

void setup()
{
  Serial.begin(9600); 
  pinMode(JoystickX, INPUT);
  pinMode(JoystickY, INPUT);
}

void loop()
{
  // Process horizontal joystick position
  int x = analogRead(JoystickX);
  int y = analogRead(JoystickY);
  
  if (x > 700 || y>700)
    Serial.println("Up");

  else if (x < 300 || y<300)
    Serial.println("down");

  else
    Serial.println("middle");

   delay(1000);
}
