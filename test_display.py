from PIL import Image
import os

# Set your screen resolution
width, height = 1920, 1080  # Replace with your display's resolution

# Create a white image
img = Image.new("RGB", (width, height), color=(255, 255, 255))
img_path = "/tmp/white_screen.jpg"
img.save(img_path, "JPEG")

# Display image using fbi (must run on console, not over SSH)
os.system(f"sudo fbi -T 1 -a {img_path}")
