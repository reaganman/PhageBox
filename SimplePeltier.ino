#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS  10

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

double hold_temp;
double hold_time;
double temperature;

int relay_pin = 4;

//PID constants
//////////////////////////////////////////////////////////
float kp = 2.75;   float ki = 0.1;   float kd = 85;
//////////////////////////////////////////////////////////

float PID_p = 0;    float PID_i = 0;    float PID_d = 0;
int threshold = 0;
//float previous_error = 0;
int error_length = 20;
float previous_errors[20] = {0};
float total_error = 10;
float PID_error = 0;
float PID_value = 0;
float d_time = 1;
int relay_is_on = 0;
float peltier_Duty = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  pinMode(relay_pin, OUTPUT);
  Serial.println("Starting up");
  //Using new Dallas Temp sensor
  sensors.begin();
  set_temp:40;
}

void loop() {
  // put your main code here, to run repeatedly:
  read_temp();
  PID_control();
  delay(200);
}


// **********************************************************
void read_temp() {
// ********************************************************** 

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
  if (temperature != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Set Temp: ");
    Serial.print(hold_temp);
    Serial.print(", ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(", ");
    Serial.print("Duty: ");
    Serial.print(peltier_Duty);
    Serial.print(", ");
    Serial.print("Relay on? ");
    if (relay_is_on == 1)
    {
      Serial.println("yes");
    }
    else
    {
      Serial.println("no");
    }
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  
}

// ********************************************************
int peltier_Window = 4;  //Duty window in seconds
//float Kp_adj = 30; //set a temperature for non-linear P gains
float heat_loss_coeff = 1;
float estimated_heat_loss = 0;
float prev_Duty = millis();
void PID_control() {
// ********************************************************** 
  //Serial.println("PID");
  PID_error = hold_temp - temperature;
  
  //Serial.println(PID_error);
  //d_time = millis() - dT_prev;   //calculate dT for PID
  
  total_error = 0;
    for (int i = 0; i < error_length-1; i++)
    {
     previous_errors[i] = previous_errors[i+1];
     total_error += previous_errors[i];
    }
    previous_errors[error_length-1] = PID_error;
  total_error += PID_error;
  total_error = constrain(total_error, -30, 30);
  
  

  //Calculate the P value
  PID_p = kp * PID_error;//+ 0.002*(temperature-20);
  //Calculate the I value in a range on +-3
  PID_i = ki*total_error;
  //Cakcykate the D value
  PID_d = kd*(PID_error - (previous_errors[error_length-2] + previous_errors[error_length-3])/2);
  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;
  // Serial.print("PID_p");
  // Serial.print(PID_p);
  // Serial.print(" PID_d");
  // Serial.println(PID_d);
  
  peltier_Duty = abs(PID_value);
  peltier_Duty = peltier_Window*peltier_Duty/10;
  peltier_Duty = constrain(peltier_Duty, 0 , peltier_Window+0.5);
  //Activate heating element if PID value is above the activation threshold

  if (millis() - prev_Duty > peltier_Window*1000)
  {
    prev_Duty = millis();
  }

  if (millis() - prev_Duty < peltier_Duty*1000)
  {
    if (PID_value > 0)
    {
      digitalWrite(relay_pin, HIGH);
      relay_is_on = 1;
    }
    else
    {
      digitalWrite(relay_pin, LOW);
      relay_is_on = 0 ;
    }
  }
  else
  {  
    digitalWrite(relay_pin, LOW);
    //Serial.println("polarity is neutral");
    relay_is_on = 0;
  }
  

  
}
