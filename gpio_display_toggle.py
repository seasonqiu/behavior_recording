import RPi.GPIO as GPIO
from PIL import Image
import os
import time

# === Config ===
PIN = 17
WIDTH, HEIGHT = 1920, 1080  # Adjust to your screen resolution

WHITE_IMG_PATH = "/home/pi/visual_stimulation/white_screen.jpg"
BLACK_IMG_PATH = "/home/pi/visual_stimulation/black_screen.jpg"

def create_solid_image(path, color):
    img = Image.new("RGB", (WIDTH, HEIGHT), color=color)
    img.save(path, "JPEG")

def show_image(path):
    os.system(f"sudo fbi -T 1 -a --noverbose --once {path}")

def clear_display():
    os.system("sudo fbi -T 1 -a --noverbose --once /dev/null")

def main():
    # === Setup GPIO ===
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    # === Create images once ===
    if not os.path.exists(WHITE_IMG_PATH):
        create_solid_image(WHITE_IMG_PATH, (255, 255, 255))
    if not os.path.exists(BLACK_IMG_PATH):
