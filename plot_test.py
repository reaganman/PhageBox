import time
from datetime import datetime

import matplotlib.pyplot as plt
import serial
from matplotlib.animation import FuncAnimation

# --- SETTINGS ---

arduino_port = "/dev/cu.usbserial-110"  # port name
baud_rate = 9600

try:
    ser = serial.Serial(arduino_port, baud_rate, timeout=1)
    print("Serial connection successful")
except Exception as e:
    print(f"Error: {e}")

# --- TIME CONTROL ---
start_time = time.time()
run_duration_sec = 30 * 60  # 30 minutes in seconds

# # --- INITIALIZE ---
ser = serial.Serial(arduino_port, baud_rate, timeout=1)
time.sleep(2)  # Wait for connection to settle
print("Connected.")

x_data = []
t_data = []
temp_data = []
timestamps = []
dir_data = []
timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
set_temp = 0.0
temp = 0.0
dire = 0.0
duty = 0.0

fig, ax = plt.subplots()
line, = ax.plot([], [], label='Temperature', color='blue')
line2, = ax.plot([], [], label='Set Temperature', color='red')
line3, = ax.plot([], [], label='Direction', color='green')
ax.relim()
ax.autoscale_view()
ax.set_title("Live Plot Test")
ax.set_xlabel("Sample")
ax.set_ylabel("TEMP (°C)")
ax.legend()
ax.grid(True)

# --- OPEN FILE EARLY ---
file = open('../../Library/CloudStorage/OneDrive-UCB-O365/SUMMER 2025/arduino_data.txt', 'w')
file.write("Timestamp,Temperature,Set Temperature\n")

def update(frame):

    global temp, set_temp, timestamp, dire, duty
    #set a time to have a set point temperature for 30min.
    elapsed_time = time.time() - start_time
    if elapsed_time > run_duration_sec:
        print(" 30 minutes complete. Closing plot and saving data.")
        file.write("Stopped after 30 minutes\n")
        file.flush()
        plt.close()
        return

    if ser.in_waiting > 0:
        raw = ser.readline().decode('utf-8').strip()
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        if "Set Temp: " in raw and "Temperature: " in raw and "Relay: " in raw:
            parts = raw.split(',')
            set_temp_str = parts[0].split("Set Temp: ")[-1].strip()
            temp_str = parts[1].split("Temperature: ")[-1].strip()
            duty_str = parts[2].split("Duty: ")[-1].strip()
            direction_str = parts[3].split("Relay on: ")[-1].strip()

            set_temp = float(set_temp_str)
            temp = float(temp_str)
            duty = float(duty_str)
            #dire = int(direction_str)

            print(f"{timestamp} | Set: {set_temp} | Temp: {temp} | Duty: {duty} | Direction: {dire}")

    timestamps.append(timestamp)
    x_data.append(frame)
    t_data.append(temp)
    temp_data.append(set_temp)
    dir_data.append(dire)


    line.set_data(x_data, t_data)
    line2.set_data(x_data, temp_data)
    line3.set_data(x_data, dir_data)  # Assuming direction is constant for the plot
    ax.relim()
    ax.autoscale_view() 

    # Write a line for each new data point
    file.write(f"{timestamp},{temp},{set_temp}\n")
    file.flush()  # Important: makes sure data is actually written to disk

    return line,line2,

try:
    #ani = FuncAnimation(fig, update, interval=500)
    ani = FuncAnimation(fig, update, interval=500, cache_frame_data=False)
    plt.show()

except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
    print("Serial connection closed.")
    if live_plot:
        plt.close(fig)
        print("Plot closed.")
    file.close()
    print("Data saved to arduino_data.txt")