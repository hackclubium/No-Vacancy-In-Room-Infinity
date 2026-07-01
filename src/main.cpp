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
                    case GameEngine::GameScreen::ROOM_LIST:
                        if (engine.ui.selectedGuestId >= 0) {
                            // Came here from Guest Detail via [R]; step back one level
                            // instead of losing the guest selection.
                            engine.ui.currentScreen = GameEngine::GameScreen::GUEST_DETAIL;
                            engine.ui.selectedRoomNumber = -1;
                            engine.ui.roomNumberInput.clear();
                        } else {
                            engine.returnToLobby();
                        }
                        break;
                    default:
                        engine.returnToLobby();
                        break;
                }
                continue;
            }
            
            if (engine.ui.currentScreen == GameEngine::GameScreen::PHONE_CALL) {
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_ESCAPE) {
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
            
            char keyChar = 0;
            if (key >= SDLK_0 && key <= SDLK_9) {
                keyChar = '0' + (key - SDLK_0);
            }

            if (engine.ui.currentScreen == GameEngine::GameScreen::ROOM_LIST) {
                if (keyChar >= '0' && keyChar <= '9') {
                    if (engine.ui.roomNumberInput.size() >= 2) {
                        engine.ui.roomNumberInput.clear();
                    }
                    engine.ui.roomNumberInput += keyChar;
                    engine.ui.selectedRoomNumber = std::stoi(engine.ui.roomNumberInput);
                } else if (key == SDLK_BACKSPACE) {
                    if (!engine.ui.roomNumberInput.empty()) {
                        engine.ui.roomNumberInput.pop_back();
                    }
                    engine.ui.selectedRoomNumber = engine.ui.roomNumberInput.empty() ?
                        -1 : std::stoi(engine.ui.roomNumberInput);
                } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    if (engine.ui.selectedRoomNumber >= 0 && engine.ui.selectedGuestId >= 0) {
                        engine.assignRoomToGuest(engine.ui.selectedRoomNumber);
                    }
                } else if (key == SDLK_g) {
                    engine.returnToLobby();
                }
                continue;
            }
            
            if (key == SDLK_a) {
                engine.answerPhone();
                continue;
            }
            
            if (key == SDLK_g) {
                engine.openRoomList();
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
            
            if (key == SDLK_r) {
                if (engine.ui.selectedGuestId >= 0) {
                    engine.openRoomList();
                }
                continue;
            }
            
            if (keyChar >= '1' && keyChar <= '9') {
                int guestIdx = keyChar - '1';
                if (guestIdx < (int)engine.state.waitingGuests.size()) {
                    engine.selectGuest(engine.state.waitingGuests[guestIdx].id);
                }
            }
        }
        
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mx = event.button.x;
            int my = event.button.y;
            
            // Click guest in lobby
            if (engine.ui.currentScreen == GameEngine::GameScreen::LOBBY) {
                int guestAreaX = 110; // deskX (50) + 60, matches Renderer::drawLobbyScene
                int guestAreaY = renderer.screenH - 170;
                for (size_t i = 0; i < engine.state.waitingGuests.size() && i < 6; i++) {
                    int gx = guestAreaX + 60 + i * 80;
                    int gy = guestAreaY;
                    if (mx >= gx && mx <= gx + 30 && my >= gy && my <= gy + 50) {
                        engine.selectGuest(engine.state.waitingGuests[i].id);
                        break;
                    }
                }
                
                // Click phone (matches Renderer::drawLobbyScene's phoneX/deskY math)
                int deskW = renderer.screenW - 250;
                int phoneX = 50 + (int)(deskW * 0.27f);
                int deskY = renderer.screenH - 260;
                if (mx >= phoneX && mx <= phoneX + 40 && my >= deskY - 25 && my <= deskY) {
                    engine.answerPhone();
                }
            }
            
            // Click room in room list
            if (engine.ui.currentScreen == GameEngine::GameScreen::ROOM_LIST) {
                int cols = 7;
                int cellW = (renderer.screenW - 60) / cols;
                int cellH = 80;
                int col = (mx - 30) / cellW;
                int row = (my - 80) / cellH;
                int idx = row * cols + col;
                if (idx >= 0 && idx < (int)engine.state.rooms.size()) {
                    engine.ui.selectedRoomNumber = engine.state.rooms[idx].number;
                    engine.ui.roomNumberInput = std::to_string(engine.state.rooms[idx].number);
                }
            }
        }
    }
    return true;
}

void mainLoop() {
    SDL_Event event;
    processInput(event);
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
        
        engine.update(deltaTime);
        renderer.render(engine);
        
        SDL_Delay(16);
    }
#endif
    
    return 0;
}
