# Embedded Stick Runner

Embedded Stick Runner is a simplified, embedded-system adaptation of the classic Chrome Dino Runner game. Instead of a dinosaur, the player controls a rectangle to jump over obstacles. The game is implemented in C for the MSP430 microcontroller.

## Features

- **Game Mechanics**: Players control a rectangle to avoid obstacles, similar to the Chrome Dino Runner.
- **Hardware**: Designed for the MSP430 microcontroller, leveraging its GPIO for input and output.
- **Low-Level Programming**: Demonstrates interrupt handling, timer utilization, and embedded C programming.

## Getting Started

### Prerequisites

- **Hardware**: MSP430 microcontroller development kit.
- **Software**: Code Composer Studio (CCS) or a compatible IDE for MSP430.
- **Tools**: USB debugger/programmer for loading firmware.

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/AaronYangello/embedded-stick-runner.git
   ```
2. Open the project in Code Composer Studio.
3. Build the project and load the firmware onto your MSP430.
4. Connect any required external components (e.g., buttons or LEDs) as per the game’s design.

### Controls

- **Jump**: Press the assigned button to make the rectangle jump over obstacles.

## Code Structure

- **`main.c`**: Contains the game logic, including input handling, obstacle generation, and game loop.
- **Interrupts**: Implements GPIO and timer-based interrupts for responsive gameplay.
- **Documentation**: Inline comments explain key parts of the code.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the Chrome Dino Runner game.
- Developed using the MSP430 microcontroller platform.

