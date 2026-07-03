# No Vacancy In Room ∞

A surreal hotel management game where every room breaks reality in a different way. Run the front desk, check guests into rooms that duplicate them, age them backwards, or simply don't exist, and try to make it through a shift without losing your mind (or your rooms).

[![Build and Deploy](https://img.shields.io/github/actions/workflow/status/hackclubium/No-Vacancy-In-Room-Infinity/deploy.yml?branch=main&label=build%20%26%20deploy&style=flat-square)](https://github.com/hackclubium/No-Vacancy-In-Room-Infinity/actions/workflows/deploy.yml)
[![Play in Browser](https://img.shields.io/badge/play-in%20browser-brightgreen?style=flat-square)](https://hackclubium.github.io/No-Vacancy-In-Room-Infinity/)
[![Top Language](https://img.shields.io/github/languages/top/hackclubium/No-Vacancy-In-Room-Infinity?style=flat-square)](https://github.com/hackclubium/No-Vacancy-In-Room-Infinity)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)](https://en.cppreference.com/w/cpp/17)
[![SDL2](https://img.shields.io/badge/SDL2-blue?style=flat-square)](https://www.libsdl.org/)
[![Last Commit](https://img.shields.io/github/last-commit/hackclubium/No-Vacancy-In-Room-Infinity?style=flat-square)](https://github.com/hackclubium/No-Vacancy-In-Room-Infinity/commits/main)
[![Repo Size](https://img.shields.io/github/repo-size/hackclubium/No-Vacancy-In-Room-Infinity?style=flat-square)](https://github.com/hackclubium/No-Vacancy-In-Room-Infinity)

## What it is

You're behind the front desk of a hotel where the rooms don't follow the rules. Guests walk up, tell you what they need, and you walk them to a room, an elevator ride and a hallway away, that may or may not deliver on that request. Some rooms duplicate whoever sleeps in them. One opens onto yesterday. Room 0 is fully booked and does not physically exist. Keep guests checked in, keep them content, and get through the shift before the clock runs out or the hotel finds a new way to embarrass you in front of the health inspector.

## Features

- **Walkable lobby, elevator, and hallways** — no menus, you move a character around with WASD and interact with whatever's nearby
- **10 guest types**, each with their own quirks, requests, and sprite art, from ordinary humans to vampires, ghosts, time travelers, and one very undercover hotel inspector
- **17 room rules**, from perfectly normal to reality-breaking (bigger on the inside, ages guests backward, duplicates its occupant, doesn't exist)
- **A live alert system** — problems happen in specific rooms, and resolving them means walking there and dealing with it, not clicking through a popup
- **Staff to dispatch** — maids, maintenance, and security who handle the fallout
- **A shift clock with real stakes** — the lobby and hallways dim toward night as your shift runs down
- **A secret inspector** who shows up undercover every few shifts and grades you on the room you gave them

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` | Move |
| `E` | Interact (talk, answer phone, call elevator, open a door) |
| `1`–`9` | Quick-select a waiting guest in the lobby |
| `ESC` | Back / cancel |
| `Enter` | Confirm / start shift / next shift |

## Playing it

The easiest way is in your browser, no install required:

**[Play No Vacancy In Room ∞](https://hackclubium.github.io/No-Vacancy-In-Room-Infinity/)**

## Building from source

Requires CMake 3.16+, a C++17 compiler, and SDL2, SDL2_ttf, and SDL2_image development libraries.

```sh
git clone https://github.com/hackclubium/No-Vacancy-In-Room-Infinity.git
cd No-Vacancy-In-Room-Infinity
cmake -B build
cmake --build build
```

### Building for the web

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```sh
emcmake cmake -B build-web
cmake --build build-web
```

## Project structure

```
src/
  game/     game state, engine loop, dialogue and phone systems
  hotel/    room rules, guest generation, staff/manager logic
  ui/       SDL2 rendering
assets/     sprites and fonts
```
