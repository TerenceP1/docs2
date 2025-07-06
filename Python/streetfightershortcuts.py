import keyboard
import time

def hdR():
    print("hdR")
    keyboard.press_and_release("down")
    time.sleep(0.2)
    keyboard.press_and_release("right")
    time.sleep(0.2)
    keyboard.press_and_release("s")

keyboard.add_hotkey("2",hdR)
keyboard.wait()