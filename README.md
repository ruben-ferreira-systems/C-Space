# 🚀 Space Game C (Ncurses Retro Shmup)

A text-mode horizontal shoot 'em up (shmup) developed purely in **C** using the **`ncurses`** library for Linux systems. Strongly inspired by the legendary *Space Impact* on the Nokia 3310 mobile phones, this project resurrects the monochrome ASCII art aesthetic while pairing it with modern real-time physical engines and fluid responsive AI.

---

## 🎮 Features & Mechanics

* **Contemporary Controls:** Fluid real-time ship movement mapping standard `W`, `A`, `S`, `D` layout with centered weapon fire inputs.
* **Dynamic Upgrades:** Horizontal movement (X-axis) and weapon systems unlock progressively as the player clears stages.
* **Procedural Generation:** Sinusoidal organic caves generated mathematically through seeds, layered seamlessly onto a real-time horizontal scrolling map buffer.
* **Component Dismantling (Boss Lvl 3):** Multi-phase combat against a 150 HP unified Mega Boss where mechanical weapon arms break off physically—triggering retro spark animations—before exposing the reactive core and firing a terminal Mega Laser beam.
* **Reactive "Bull Mode" AI:** Bosses actively monitor damage thresholds. Getting shot disrupts their predictable flight paths, forcing immediate aggressive diagonal dashes directly toward the player's coordinates.
* **Retro Audio Engine:** Hard-coded audio hooks synthesizing pure sound waves (`sine`, `square`, `brownnoise`) natively through the Linux terminal shell.
* **Data Persistence:** Dedicated binary state tracking for save/load files (`save.dat`) alongside an open textual leaderboards file (`highscores.txt`).

---

## 📂 Project Architecture (Modular Structure)

The game engine is split into 6 dedicated modules to ensure clean code encapsulation and prevent bloated source structures:


| Module | Files | Description |
| :--- | :--- | :--- |
| **Core Loop** | `main.c` | Controls the global infinite retry cycle (*outer loop*) and precise frame timings (*inner loop*) driven by `usleep`. |
| **Physics Engine** | `space.h` / `space.c` | Handles real-time asynchronous inputs, projectile tracking, collision loops, and basic tunnel enemy `x` behaviors. |
| **Level Manager** | `levels.h` / `levels.c` | Controls dynamic map memory boundaries, safety spacing buffers, and procedural generation formulas. |
| **Artificial Intelligence** | `boss.h` / `boss.c` | Isolates Finite State Machines (FSM), attack matrices (Bull Rush, Mega Laser), and raw ASCII boss graphics. |
| **Graphics Unit** | `draw.h` / `draw.c` | Purely visual rendering pipeline handling level backgrounds, unified HUD states, and custom screen transitions. |
| **Interface & Storage** | `menu.h` / `menu.c` | Keyboard-driven menus (Play, Load, Highscores, Exit) and direct file serialization procedures. |

---

## 🛠️ Build and Execution

The project comes packed with an optimized **`Makefile`** that automates building, object binding, and optimization routines, linking essential components (`-lncurses`, `-lm`).

### Prerequisites (Ubuntu/Debian)
Ensure you have the GNU Compiler Collection (`gcc`), compilation utilities, the `ncurses` development headers, and the standard Linux synthesis audio framework installed:
```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev sox
```
*(Note: The `sox` package is highly recommended to handle terminal audio synthesis hooks seamlessly).*

### Build Commands
Run the following tasks inside the project's root folder using your Linux terminal:

* **Compile the game:**
  ```bash
  make
  ```
* **Run the executable:**
  ```bash
  ./space
  ```
* **Remove intermediate object files (`.o`):**
  ```bash
  make clean
  ```
* **Full wipe (Objects + Executable binary):**
  ```bash
  make fclean
  ```
* **Complete re-build:**
  ```bash
  make re
  ```

---

## 📊 Engine Technical Specifications

* **Screen Resolution:** Locked at `40x15` text cells to preserve taptic retro arcade parity.
* **Map Length Buffer:** Allocates up to `1150` horizontal columns in volatile memory with window viewport slicing.
* **Engine Refresh Rate:** Governed by a fixed clock interval delay of `30000` microseconds (`~33 FPS`), maintaining consistent physical intervals across variable CPU cycles.
* **Player Stat Ceiling:** Balanced gameplay limiting maximum active ship reserves strictly up to 9 lives.

---
⭐ *Developed in C as a tribute to early 2000s handheld gaming engineering.*
