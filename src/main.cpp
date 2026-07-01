#include "game/engine.hpp"
#include "ui/renderer.hpp"
#include <SDL2/SDL.h>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace game_engine;
using namespace ui_renderer;

GameEngine engine;
Renderer renderer;

bool processInput(SDL_Event& event) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return false;

        if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;

            if (engine.ui.currentScreen == GameEngine::GameScreen::TITLE) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE) {
                    engine.startGame();
                }
                continue;
            }

            if (engine.ui.currentScreen == GameEngine::GameScreen::END_OF_DAY) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    engine.nextDay();
                }
                continue;
            }

            if (key == SDLK_ESCAPE) {
                switch (engine.ui.currentScreen) {
                    case GameEngine::GameScreen::PHONE_CALL:
                        engine.hangUpPhone();
                        break;
                    case GameEngine::GameScreen::ALERT_POPUP:
                        engine.handleAlertAction(engine.ui.selectedAlertIndex, "IGNORE");
                        break;
                    case GameEngine::GameScreen::GUEST_DETAIL:
                        engine.ui.currentScreen = GameEngine::GameScreen::LOBBY;
                        break;
                    case GameEngine::GameScreen::ELEVATOR_MENU:
                        engine.ui.currentScreen = GameEngine::GameScreen::LOBBY;
                        break;
                    case GameEngine::GameScreen::HALLWAY:
                        engine.exitHallwayToLobby();
                        break;
                    default:
                        engine.returnToLobby();
                        break;
                }
                continue;
            }

            if (engine.ui.currentScreen == GameEngine::GameScreen::PHONE_CALL) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_e) {
                    engine.hangUpPhone();
                }
                continue;
            }

            if (engine.ui.currentScreen == GameEngine::GameScreen::ALERT_POPUP) {
                if (key == SDLK_1) engine.handleAlertAction(engine.ui.selectedAlertIndex, "SEND_SECURITY");
                else if (key == SDLK_2) engine.handleAlertAction(engine.ui.selectedAlertIndex, "SEND_MAINTENANCE");
                else if (key == SDLK_3) engine.handleAlertAction(engine.ui.selectedAlertIndex, "SEND_MAID");
                continue;
            }

            if (engine.ui.currentScreen == GameEngine::GameScreen::ELEVATOR_MENU) {
                if (key == SDLK_w || key == SDLK_UP) engine.moveElevatorSelection(-1);
                else if (key == SDLK_s || key == SDLK_DOWN) engine.moveElevatorSelection(1);
                else if (key == SDLK_e || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    engine.confirmFloorSelection(renderer.screenW);
                }
                continue;
            }

            char keyChar = 0;
            if (key >= SDLK_0 && key <= SDLK_9) {
                keyChar = '0' + (key - SDLK_0);
            }

            // LOBBY, GUEST_DETAIL (overlay), and HALLWAY fall through to here.

            if (key == SDLK_e) {
                engine.interact(renderer.screenW, renderer.screenH);
                continue;
            }

            if (key == SDLK_p) {
                int alertIdx = engine.getFirstUnhandledAlert();
                if (alertIdx >= 0) {
                    engine.ui.selectedAlertIndex = alertIdx;
                    engine.ui.currentScreen = GameScreen::ALERT_POPUP;
                }
                continue;
            }

            if (key == SDLK_TAB) {
                int startIdx = engine.ui.selectedAlertIndex + 1;
                if (startIdx >= (int)engine.state.activeAlerts.size()) startIdx = 0;
                for (int i = 0; i < (int)engine.state.activeAlerts.size(); i++) {
                    int idx = (startIdx + i) % engine.state.activeAlerts.size();
                    if (!engine.state.activeAlerts[idx].handled) {
                        engine.ui.selectedAlertIndex = idx;
                        engine.ui.currentScreen = GameScreen::ALERT_POPUP;
                        break;
                    }
                }
                continue;
            }

            if (keyChar >= '1' && keyChar <= '9' && engine.ui.currentScreen == GameEngine::GameScreen::LOBBY) {
                int guestIdx = keyChar - '1';
                if (guestIdx < (int)engine.state.waitingGuests.size()) {
                    engine.ui.selectedGuestId = engine.state.waitingGuests[guestIdx].id;
                    engine.ui.currentScreen = GameEngine::GameScreen::GUEST_DETAIL;
                }
            }
        }
    }
    return true;
}

void updateMovementInput(float deltaTime) {
    const Uint8* ks = SDL_GetKeyboardState(NULL);
    float dx = 0.0f, dy = 0.0f;
    if (ks[SDL_SCANCODE_W]) dy -= 1.0f;
    if (ks[SDL_SCANCODE_S]) dy += 1.0f;
    if (ks[SDL_SCANCODE_A]) dx -= 1.0f;
    if (ks[SDL_SCANCODE_D]) dx += 1.0f;

    // Always call this (even with zero input) so the position stays clamped
    // to the screen if the window is resized while the player is standing still.
    engine.movePlayer(dx, dy, deltaTime, renderer.screenW, renderer.screenH);
}

void mainLoop() {
    SDL_Event event;
    processInput(event);
    updateMovementInput(0.016f);
    engine.update(0.016f); // ~60fps
    renderer.render(engine);
}

int main(int argc, char* argv[]) {
    SDL_Log("No Vacancy in Room Infinity - Starting...");

    if (!renderer.init()) {
        SDL_Log("Failed to initialize renderer!");
        return 1;
    }

    engine.init();
    SDL_Log("Game initialized. Lobby is ready.");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        running = processInput(event);

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        if (deltaTime > 0.1f) deltaTime = 0.1f;

        updateMovementInput(deltaTime);
        engine.update(deltaTime);
        renderer.render(engine);

        SDL_Delay(16);
    }
#endif

    return 0;
}
