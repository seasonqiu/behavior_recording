from PIL import Image

# Replace with your screen resolution
width, height = 1024, 600 # in bash: fbset to acquire the dimension of the screen

# Create all-white image (255, 255, 255 is white)
white_image = Image.new("RGB", (width, height), color=(255, 255, 255))

# Save as JPEG
white_image.save("white_fullscreen.jpg", "JPEG")
