import tkinter as tk
from PIL import Image, ImageTk
import os
import sys

# === Configuration ===
IMAGE_PATH = "test_image.jpg"  # Replace with your image path
FULLSCREEN = True              # Set to False if you want windowed display

# === Check image file ===
if not os.path.exists(IMAGE_PATH):
    print(f"Error: '{IMAGE_PATH}' not found.")
    sys.exit(1)

# === Create GUI ===
root = tk.Tk()

if FULLSCREEN:
    root.attributes('-fullscreen', True)
else:
    root.geometry("800x600")  # Windowed fallback

root.configure(background='black')

# === Load and scale image ===
img = Image.open(IMAGE_PATH)

# Match screen resolution
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()
img = img.resize((screen_width, screen_height), Image.ANTIALIAS)

tk_img = ImageTk.PhotoImage(img)

label = tk.Label(root, image=tk_img)
label.pack(expand=True)

# === Key bindings ===
def close(event=None):
    root.destroy()

root.bind("<Escape>", close)  # Press Esc to exit

# === Run ===
root.mainloop()
