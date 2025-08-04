from PIL import Image
import os

# Screen resolution
width, height = 1920, 1080  # Change as needed

# Create white image
img = Image.new("RGB", (width, height), color=(255, 255, 255))
img_path = "/tmp/white_screen.jpg"
img.save(img_path, "JPEG")

# Display image without caption bar
os.system(f"sudo fbi -T 1 -a --noverbose --once {img_path}")
