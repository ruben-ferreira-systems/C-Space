# 🚀 Space Game C

A terminal-based horizontal shoot 'em up written in C using the `ncurses` library.

Originally started as a small teaching project while preparing programming exercises, it gradually evolved into a complete retro-inspired game featuring procedural levels, boss fights, save/load support, and a modular architecture.

Inspired by classic games such as *Space Impact* on the Nokia 3310.

---

## Features

* Real-time gameplay using `ncurses`
* Multiple levels and boss encounters
* Procedurally generated cave sections
* Save and load functionality
* High score tracking
* Progressive player upgrades
* Modular codebase split across multiple components

---

## Project Structure

| Module    | Files                   | Responsibility                              |
| --------- | ----------------------- | ------------------------------------------- |
| Core Loop | `main.c`                | Main game loop and timing                   |
| Physics   | `space.c` / `space.h`   | Player movement, projectiles and collisions |
| Levels    | `levels.c` / `levels.h` | Procedural generation and map handling      |
| Bosses    | `boss.c` / `boss.h`     | Boss logic and attack patterns              |
| Rendering | `draw.c` / `draw.h`     | Screen drawing and HUD                      |
| Menus     | `menu.c` / `menu.h`     | Menus, saves and highscores                 |

---

## Requirements

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```

---

## Build

Compile:

```bash
make
```

Run:

```bash
./space
```

Clean object files:

```bash
make clean
```

Remove objects and executable:

```bash
make fclean
```

Rebuild:

```bash
make re
```

---

## Technical Notes

* Written in ANSI C
* Uses `ncurses` for rendering
* Fixed timestep game loop
* Procedural cave generation
* File-based persistence for saves and highscores
* Modular architecture with separate gameplay systems

---

## Why This Project?

This project was primarily developed as a way to explore game architecture, procedural generation, state management, and modular design in C while creating examples for programming students.

It later grew into a complete playable game.

---

Developed as a tribute to early handheld gaming and terminal-based programming.

<img width="333" height="273" alt="image" src="https://github.com/user-attachments/assets/75d178f6-27d0-4f60-866f-c2f40c339214" />
