
#include "irDecoderV2.h"

const int IR_pin = 10;

int code = 0;

void setup()
{
  Serial.begin(9600);  
  ir_begin(IR_pin);  // Start the decoder
}

void loop()
{
  code = ir_data();  // Check for IR code
  
  if(code > 0)  // If a code is available...
  {
    Serial.print("Code: "); // Print it
    Serial.println(code);
  }
  
  else if(code == -1)  // If its a repeat
  {
    Serial.println("Repeat");  // Print it
  }
  
  delay(250);  // Prevent clogging up serial monitor
}
