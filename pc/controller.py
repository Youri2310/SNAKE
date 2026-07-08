import argparse
import sys

import serial
from pynput import keyboard

KEYS = {"z", "q", "s", "d"}


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Controleur Snake ESP32 (USB serie)")
    parser.add_argument("--port", required=True, help="Port serie (ex: COM6, /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200)
    return parser.parse_args(argv)


def run(port, baud):
    link = serial.Serial(port, baud, timeout=1)
    print(f"Connecte a {port}. ZQSD pour bouger, Echap pour quitter.")

    def on_press(key):
        if key == keyboard.Key.esc:
            return False
        char = getattr(key, "char", None)
        if char and char.lower() in KEYS:
            link.write(f"{char.lower()}\n".encode("ascii"))
        return None

    with keyboard.Listener(on_press=on_press) as listener:
        listener.join()
    link.close()


def main(argv=None):
    args = parse_args(argv)
    run(args.port, args.baud)
    return 0


if __name__ == "__main__":
    sys.exit(main())
