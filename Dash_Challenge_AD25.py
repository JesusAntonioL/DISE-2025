import tkinter as tk
import math
import serial
import threading

class TemperatureBar(tk.Canvas):
    def __init__(self, parent, width=300, height=50, min_temp=-20, max_temp=150, **kwargs):
        super().__init__(parent, width=width, height=height, **kwargs)
        self.width = width
        self.height = height
        self.min_temp = min_temp
        self.max_temp = max_temp
        self.temperature = 20  # Default temperature
        
        # Create the temperature bar
        self.bar_width = width - 40
        self.bar_height = 20
        self.bar_x = 20
        self.bar_y = 15
        
        # Draw the bar background
        self.create_rectangle(self.bar_x, self.bar_y, 
                             self.bar_x + self.bar_width, self.bar_y + self.bar_height, 
                             fill="lightgray", outline="black")
        
        # Draw temperature markers
        self.draw_markers()
        
        # Create temperature indicator
        self.indicator = self.create_rectangle(0, 0, 5, self.bar_height + 10, fill="red", outline="")
        
        # Temperature label
        self.temp_label = self.create_text(width // 2, self.bar_y + self.bar_height + 15, 
                                          text="20°C", font=("Arial", 12), fill="white")
        
        self.update_indicator()
    
    def draw_markers(self):
        # Draw major markers and labels every 20 degrees
        for temp in range(self.min_temp, self.max_temp + 1, 20):
            x = self.bar_x + (temp - self.min_temp) / (self.max_temp - self.min_temp) * self.bar_width
            self.create_line(x, self.bar_y, x, self.bar_y + self.bar_height, fill="black", width=1)
            self.create_text(x, self.bar_y - 10, text=str(temp), font=("Arial", 8), fill="white")
        
        # Draw minor markers every 10 degrees
        for temp in range(self.min_temp, self.max_temp + 1, 10):
            if temp % 20 != 0:  # Skip major markers
                x = self.bar_x + (temp - self.min_temp) / (self.max_temp - self.min_temp) * self.bar_width
                self.create_line(x, self.bar_y + 5, x, self.bar_y + self.bar_height - 5, fill="black", width=1)
    
    def update_indicator(self):
        # Calculate position based on temperature
        x_pos = self.bar_x + (self.temperature - self.min_temp) / (self.max_temp - self.min_temp) * self.bar_width
        
        # Update indicator position
        self.coords(self.indicator, 
                   x_pos - 2, self.bar_y - 5, 
                   x_pos + 2, self.bar_y + self.bar_height + 5)
        
        # Update temperature label
        self.itemconfig(self.temp_label, text=f"{self.temperature}°C")
        
        # Change color based on temperature
        if self.temperature < 0:
            color = "blue"
        elif self.temperature < 50:
            color = "green"
        elif self.temperature < 100:
            color = "orange"
        else:
            color = "red"
        
        self.itemconfig(self.indicator, fill=color)
    
    def set_temperature(self, temp):
        self.temperature = max(min(temp, self.max_temp), self.min_temp)
        self.update_indicator()

class RPMGauge(tk.Canvas):
    def __init__(self, parent, width=300, height=300, max_rpm=1000, bg_color="white", tick_color="black", **kwargs):
        super().__init__(parent, width=width, height=height, bg=bg_color, **kwargs)
        self.width = width
        self.height = height
        self.max_rpm = max_rpm
        self.bg_color = bg_color
        self.tick_color = tick_color
        self.rpm = 0
        self.center = (width // 2, height // 2)
        self.radius = min(width, height) // 2 - 20
        self.create_oval(20, 20, width-20, height-20, outline="black", width=2)
        self.draw_ticks()
        self.needle = None
        self.update_needle()

    def draw_ticks(self):
        for i in range(0, self.max_rpm + 1, 200):
            angle = math.radians(225 - (270 * i / self.max_rpm))
            x1 = self.center[0] + (self.radius - 20) * math.cos(angle)
            y1 = self.center[1] - (self.radius - 20) * math.sin(angle)
            x2 = self.center[0] + self.radius * math.cos(angle)
            y2 = self.center[1] - self.radius * math.sin(angle)
            self.create_line(x1, y1, x2, y2, fill=self.tick_color, width=2)
            label_x = self.center[0] + (self.radius - 40) * math.cos(angle)
            label_y = self.center[1] - (self.radius - 40) * math.sin(angle)
            self.create_text(label_x, label_y, text=str(i), font=("Arial", 10), fill=self.tick_color)

    def update_needle(self):
        if self.needle:
            self.delete(self.needle)
        angle = math.radians(225 - (270 * self.rpm / self.max_rpm))
        x = self.center[0] + (self.radius - 40) * math.cos(angle)
        y = self.center[1] - (self.radius - 40) * math.sin(angle)
        self.needle = self.create_line(self.center[0], self.center[1], x, y, fill="red", width=4)

    def set_rpm(self, rpm):
        self.rpm = min(max(rpm, 0), self.max_rpm)
        self.update_needle()

class DoorLockStatus(tk.Frame):
    def __init__(self, parent, **kwargs):
        super().__init__(parent, **kwargs)
        self.locked = True
        self.status_label = tk.Label(self, text="Doors Locked", font=("Arial", 16), bg="green", fg="white", width=20)
        self.status_label.pack(pady=10)

    def update_status(self, locked):
        self.locked = locked
        if self.locked:
            self.status_label.config(text="Doors Locked", bg="green")
        else:
            self.status_label.config(text="Doors Unlocked", bg="red")

class RPMApp(tk.Tk):
    def __init__(self, serial_port="COM6", baud_rate=115600):
        super().__init__()
        self.title("Car Instruments")
        self.geometry("400x600")  # Increased height to accommodate temperature bar

        self.gauge = RPMGauge(self, width=300, height=300, max_rpm=1000, bg_color="lightblue", tick_color="darkblue")
        self.gauge.pack(pady=10)

        # Add temperature bar
        self.temp_bar = TemperatureBar(self, width=300, height=80)
        self.temp_bar.pack(pady=10)

        self.door_lock_status = DoorLockStatus(self)
        self.door_lock_status.pack(pady=10)

        self.serial_port = serial.Serial(serial_port, baud_rate, timeout=1)
        self.after(100, self.read_serial_data)

    def read_serial_data(self):
        if self.serial_port.in_waiting > 0:
            try:
                data = self.serial_port.readline().decode('utf-8').strip()
                # Assuming data format is "rpm,door_status,temperature"
                values = data.split(',')
                rpm = int(values[0])
                door_status = int(values[1])
                temperature = int(values[2]) if len(values) > 2 else 20  # Default to 20 if no temp data
                
                self.gauge.set_rpm(rpm)
                self.door_lock_status.update_status(door_status == 1)
                self.temp_bar.set_temperature(temperature)
            except (ValueError, IndexError):
                pass  # Ignore errors in case of corrupt data
        self.after(100, self.read_serial_data)

if __name__ == "__main__":
    # Replace '/dev/ttyUSB0' with your actual serial port (e.g., 'COM3' on Windows)
    app = RPMApp(serial_port='COM4', baud_rate=115600)
    app.mainloop()