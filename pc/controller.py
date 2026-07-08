import argparse
import sys

import serial
from pynput import keyboard

KEYMAP = {
    "z": "z",
    "q": "q",
    "s": "s",
    "d": "d",
    "i": "i",
    "j": "j",
    "k": "k",
    "l": "l",
    "&": "1",
    "é": "2",
    '"': "3",
}


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Controleur Snake ESP32 (USB serie)")
    parser.add_argument("--port", required=True, help="Port serie (ex: COM6, /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200)
    return parser.parse_args(argv)


def run(port, baud):
    link = serial.Serial(port, baud, timeout=1)
    print(f"Connecte a {port}. P1=ZQSD P2=IJKL, menu &/e, \"=retour menu, Echap pour quitter.")

    def on_press(key):
        if key == keyboard.Key.esc:
            return False
        char = getattr(key, "char", None)
        if char:
            command = KEYMAP.get(char.lower())
            if command:
                link.write(f"{command}\n".encode("ascii"))
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
