#include "TemperatureModule.h"
#include "GPIO_Control.h"


// Defined in header
TemperatureModule::TemperatureModule(uint8_t relay_pin, uint8_t tempsense_pin, volatile int *timer)
{
    // start in off state
    current_state = STOPPED;
    // update temperature sensor
    register OneWire *oneWire = new OneWire(tempsense_pin);
    temp_sensor.setOneWire(oneWire);
    // temp_sensor.setResolution(12);
    temp_sensor.setWaitForConversion(false);
    // temp_sensor = sensors;
    temp_sensor.begin();
    // update temperature sensor
    relayPin = relay_pin;
    // assign timer
    internal_timer = timer;
}

// Defined in header
// void TemperatureModule::update_states() 
// {
//     if (*internal_timer >= get_currentTimeLen()) {

//         go2nextstate();
//         float curr_temp = getTemp();
//         float desired_temp = get_desiredTemp();

//         // Preheat or precool if necessary
//         // if (curr_temp > desired_temp) 
//         // {
//         //     Serial.print("Current state: ");
//         //     Serial.print(current_state);
//         //     Serial.print("\n");
//         //     Serial.println("Precooling\n");
//         //     heater_off();
//         // } 
        
//         // else if (curr_temp < desired_temp) 
//         // {
//         //     Serial.print("Current state: ");
//         //     Serial.print(current_state);
//         //     Serial.print("\n");
//         //     Serial.print("Preheating\n");
//         //     heater_on();
//         // }

//         if (abs(curr_temp - desired_temp) <= TEMP_TOLERANCE) {
//             Serial.print("TIME PASSED = ");
//             Serial.print(*internal_timer);
//             Serial.println(" seconds \n");

//             if (current_state == ELONGATE) {
//                 number_cycles_used++;
//                 Serial.println("Switching to ELONGATE \n");
//             } else if (current_state == ANNEAL) {
//                 Serial.println("Switching to ANNEAL \n");
//             } else if (current_state == DENATURE) {
//                 Serial.println("Switching to DENATURE \n");
//             }
//             reset_timer(internal_timer);
//         }
//     }

//     if (number_cycles_used >= cycle_count) {
//         stop_pcr();
//     }
// }


bool TemperatureModule::preheat()
{
    float curr_temp = getTemp();
    float next_temp = state_table[state_table[current_state].next_state].temp;
    
    while (abs(curr_temp - next_temp) > TEMP_TOLERANCE) // Exit when within tolerance
    {
        curr_temp = getTemp(); // Continuously update temperature

        if (curr_temp > next_temp) 
        {
            Serial.print("Current temp: ");
            Serial.print(curr_temp);
            Serial.print("\nNext state temp: ");
            Serial.print(next_temp);
            Serial.print("\nPrecooling\n");
            heater_off();
        } 
        else if (curr_temp < next_temp) 
        {
            Serial.print("Current temp: ");
            Serial.print(curr_temp);
            Serial.print("\nNext state temp: ");
            Serial.print(next_temp);
            Serial.print("\nPreheating\n");
            heater_on();
        }
        delay(500); // Allow time for temperature to adjust
    }

    Serial.println("Preheating complete!");
    return true; // Return true when preheated
}


void TemperatureModule::update_states()
{
    if (*internal_timer >= get_currentTimeLen())
    {
        // Ensure preheating before transitioning
        if (preheat()) 
        {
            go2nextstate();
            Serial.print("TIME PASSED = ");
            Serial.print(*internal_timer);
            Serial.println(" seconds \n");

            if (current_state == ELONGATE)
            {
                number_cycles_used++;
                Serial.println("Switching to ELONGATE \n");
            }
            else if (current_state == ANNEAL)
            {
                Serial.println("Switching to ANNEAL \n");
            }
            else if (current_state == DENATURE)
            {
                Serial.println("Switching to DENATURE \n");
            }

            reset_timer(internal_timer);
        }
        else
        {
            Serial.println("Preheating not complete, staying in current state.");
        }
    }
    if (number_cycles_used >= cycle_count)
    {
        stop_pcr();
    }
}


// Defined in header
float TemperatureModule::getTemp()
{
    temp_sensor.requestTemperatures();
    delay(100); // essential for letting temp 1-wire work
    current_temperature = temp_sensor.getTempCByIndex(0);
    return current_temperature;
}

// Defined in header
void TemperatureModule::toggle_relay()
{
    toggle_pin(relayPin); // from GPIO
}

// Defined in header
void TemperatureModule::heater_on()
{
    Serial.print("Heater ON \n");
    toggle_pin_off(relayPin); // from GPIO module
}

// Defined in header
void TemperatureModule::heater_off()
{
    Serial.print("Heater OFF \n");
    toggle_pin_on(relayPin); // from GPIO module
}

// Defined in header
void TemperatureModule::update_state_table(pcr_state state, int time, int temp)
{
    state_table[state].time = time;
    state_table[state].temp = temp;
}

// Defined in header
int TemperatureModule::get_currentTimeLen()
{
    return state_table[current_state].time;
}

// Defined in header
float TemperatureModule::get_desiredTemp()
{
    return state_table[current_state].temp;
}

// Defined in header
void TemperatureModule::set_cycle_count(int cycle_count_in)
{
    cycle_count = cycle_count_in;
    Serial.println("cycle_count is set \n");
    Serial.println(cycle_count);
}

// Defined in header
void TemperatureModule::start_pcr()
{
    reset_timer(internal_timer);
    number_cycles_used = 0;
    current_state = DENATURE;
}

// Defined in header
void TemperatureModule::stop_pcr()
{
    Serial.print("stop called");
    heater_off();
    current_state = STOPPED;
}