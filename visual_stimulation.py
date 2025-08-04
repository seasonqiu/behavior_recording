import RPi.GPIO as GPIO
import time
from PIL import Image, ImageTk
import tkinter as tk

# Settings
GPIO_PIN = 17  # Change to your GPIO pin number
IMAGE_PATH = "/home/pi/visual_stimulation/image.jpg"  # Path to your image file

# Setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(GPIO_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# Tkinter setup
root = tk.Tk()
root.attributes("-fullscreen", True)
root.configure(background='black')
canvas = tk.Canvas(root, bg='black', highlightthickness=0)
canvas.pack(fill=tk.BOTH, expand=True)

# Load image
image = Image.open(IMAGE_PATH)
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()
image = image.resize((screen_width, screen_height), Image.ANTIALIAS)
tk_image = ImageTk.PhotoImage(image)
image_item = canvas.create_image(0, 0, anchor='nw', image=tk_image)
canvas.itemconfigure(image_item, state='hidden')  # Start hidden

def update_display():
    if GPIO.input(GPIO_PIN) == GPIO.HIGH:
        canvas.itemconfigure(image_item, state='normal')  # Show image
    else:
        canvas.itemconfigure(image_item, state='hidden')  # Hide image
    root.after(100, update_display)  # Check again in 100ms

# Start monitoring
root.after(0, update_display)
root.mainloop()

# Cleanup on exit
GPIO.cleanup()
