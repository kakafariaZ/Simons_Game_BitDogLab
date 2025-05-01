# 🧠 Simon's Game on BitDogLab (Raspberry Pi Pico W)

This project implements the classic **Simon's Game** using the **BitDogLab** development board, based on the **Raspberry Pi Pico W**.

The game challenges the player's memory by reproducing sequences of lights and sounds that progressively increase in difficulty. All logic is written in **C**, leveraging the embedded features of the platform such as GPIOs, LED/button control, and sound output.

## 🎮 How It Works

- A random sequence of lights and sounds is generated.
- The player must repeat the sequence by pressing the corresponding buttons.
- Each round adds a new element to the sequence.
- If the player makes a mistake, the game resets.

## 🔧 Features

- Developed in C using the Raspberry Pi Pico SDK
- Controls 3 LEDs (or RGB LEDs)
- Reads input from 3 physical buttons
- Optional buzzer for sound feedback
- Simple and responsive gameplay loop

## 🛠️ Hardware Requirements

- BitDogLab board with Raspberry Pi Pico W
- 3 buttons (corresponding to colors)
- 3 LEDs (red, green, blue)
- 1 piezo buzzer (optional)

## 📦 Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/your-username/simons-game-pico.git
