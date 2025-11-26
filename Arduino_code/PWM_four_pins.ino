#include <string.h>

int pin_l;

struct VALVE{
  int pin;
  int ison = 0;
  int PWMperc = 50;
  long period = 10000;
  unsigned long lastswitch;
};

VALVE* Valve_Arr =0;
int Valve_Arr_size = 0;

char receivedData[25];  // Buffer to store incoming serial data

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Ready...");
  
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 2;i<6;i++){
      pinMode(i,OUTPUT);
      delay(10);
      digitalWrite(i,LOW);
      createValve(i,0,50,10000);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  
}

void loop() {
  // Check if serial data is available
  if (Serial.available() > 0) {
    int bytesRead = Serial.readBytesUntil(';', receivedData, sizeof(receivedData) - 1);
    receivedData[bytesRead] = '\0';  // Ensure null termination

    // Look for "PWM" in the received string
    if (strncmp(receivedData, "PWM", 3) == 0) {
      pin_l = atoi(receivedData + 3);    // Extract pin number
      // Ensure valid pin
      if (pin_l < 2 || pin_l > 5) {
        Serial.println("Invalid pin! Must be 2, 3, 4 or 5.");
        return;
      }
      int i = findValve(pin_l);
      if (i>=0){
          Valve_Arr[i].pin =pin_l;
          Valve_Arr[i].ison = atoi(receivedData + 7);  
          Valve_Arr[i].PWMperc =atoi(receivedData + 9);
          Valve_Arr[i].period = atoi(receivedData + 13);
          Serial.print(" PWM: "); Serial.print(Valve_Arr[i].PWMperc);
      }
    }
  }
  update_state();
}


void createValve(int pin, bool ison, int PWMperc,long period)
{
  Valve_Arr_size++;

  if(Valve_Arr !=0)
  {
    Valve_Arr = (VALVE*) realloc(Valve_Arr, Valve_Arr_size * sizeof(VALVE));
    }
  else
  {
    Valve_Arr = (VALVE*) malloc(Valve_Arr_size * sizeof(VALVE));
  }  

  Valve_Arr[Valve_Arr_size-1].pin =pin;
  Valve_Arr[Valve_Arr_size-1].ison =ison;
  Valve_Arr[Valve_Arr_size-1].PWMperc =PWMperc;
  Valve_Arr[Valve_Arr_size-1].period = period;
  Valve_Arr[Valve_Arr_size-1].lastswitch = millis();
}

void update_state()
{
  // PWM Logic
  unsigned long now = millis();
  long elapsed;

  for(int i = 0;i<Valve_Arr_size;i++)
  {
    if(Valve_Arr[i].ison==1)
    {
      elapsed = now - Valve_Arr[i].lastswitch;

      if (elapsed > Valve_Arr[i].period) 
      {
         Valve_Arr[i].lastswitch = now;
      } else if (elapsed < (Valve_Arr[i].period * Valve_Arr[i].PWMperc) / 100) 
      {
        digitalWrite(Valve_Arr[i].pin, HIGH);
      } else 
      {
       digitalWrite(Valve_Arr[i].pin, LOW);

      }
    }else{
      Valve_Arr[i].lastswitch = now;
    }
  }  
}


int findValve(int pin)
{
  for(int i=0;i<Valve_Arr_size;i++)
  {
   if(Valve_Arr[i].pin==pin)
   {
    return i;
   }
  }
  return -1;
}
