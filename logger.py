import serial
import csv
import time

# CHANGE THIS to your ESP32's COM port
COM_PORT = 'COM5' 
BAUD_RATE = 115200
LABEL = 'severe_turbulence' # Change this for each recording!
DURATION_SECONDS = 10

print(f"Connecting to {COM_PORT}...")
ser = serial.Serial(COM_PORT, BAUD_RATE)

filename = f"{LABEL}_{int(time.time())}.csv"
with open(filename, 'w', newline='') as f:
    writer = csv.writer(f)
    # Edge Impulse explicitly requires this exact header for time-series data
    writer.writerow(['timestamp', 'roll', 'pitch', 'yaw'])
    
    print(f"Recording {LABEL} for {DURATION_SECONDS} seconds... SHAKE NOW!")
    start_time = time.time()
    
    while time.time() - start_time < DURATION_SECONDS:
        try:
            line = ser.readline().decode('utf-8').strip()
            data = line.split(',')
            
            # ESP-IDF prints boot logs. We only want the lines that have exactly 4 numbers
            if len(data) == 4 and data[0].replace('.','',1).isdigit():
                writer.writerow(data)
                print(f"Saved: {data}") # Print to terminal so you know it's working
        except Exception as e:
            pass

print(f"Done! Saved as {filename}")
ser.close()