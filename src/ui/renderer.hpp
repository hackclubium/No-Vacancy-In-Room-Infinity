#pragma once

#include "game/engine.hpp"
#include "hotel/rooms.hpp"
#include "hotel/guests.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace game_engine;

struct UIRect {
    int x, y, w, h;
};

struct UIColor {
    Uint8 r, g, b, a = 255;
};

namespace ui_renderer {

// Color palette - moody hotel vibe
const UIColor COLOR_BG         = {20, 16, 28, 255};
const UIColor COLOR_PANEL_BG   = {28, 24, 38, 255};
const UIColor COLOR_PANEL_BORDER = {60, 50, 80, 255};
const UIColor COLOR_TEXT       = {220, 210, 200, 255};
const UIColor COLOR_TEXT_DIM   = {140, 130, 120, 255};
const UIColor COLOR_TEXT_BRIGHT = {255, 248, 235, 255};
const UIColor COLOR_ACCENT     = {180, 150, 90, 255};
const UIColor COLOR_ACCENT2    = {120, 100, 180, 255};
const UIColor COLOR_WARN       = {210, 130, 50, 255};
const UIColor COLOR_DANGER     = {210, 60, 50, 255};
const UIColor COLOR_SUCCESS    = {90, 170, 100, 255};
const UIColor COLOR_HIGHLIGHT  = {200, 180, 100, 255};
const UIColor COLOR_BUTTON_BG  = {45, 40, 60, 255};
const UIColor COLOR_BUTTON_HOVER = {65, 55, 85, 255};
const UIColor COLOR_LOBBY_FLOOR = {35, 28, 45, 255};
const UIColor COLOR_DESK       = {55, 40, 30, 255};
const UIColor COLOR_ELEVATOR   = {50, 50, 60, 255};
const UIColor COLOR_KEY_BOARD  = {45, 35, 25, 255};
const UIColor COLOR_GUEST_PH   = {80, 70, 90, 255};
const UIColor COLOR_PHONE      = {40, 35, 30, 255};
const UIColor COLOR_PLAYER     = {230, 195, 110, 255};
const UIColor COLOR_DOOR       = {70, 55, 45, 255};

class Renderer {
public:
    SDL_Window* window = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    TTF_Font* font = nullptr;
    TTF_Font* fontSmall = nullptr;
    TTF_Font* fontMono = nullptr;
    TTF_Font* fontTitle = nullptr;
    int screenW = 1280;
    int screenH = 720;

    int tick = 0;

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;
        if (TTF_Init() < 0) return false;

        window = SDL_CreateWindow("No Vacancy in Room Infinity",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            screenW, screenH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window) return false;

        SDL_SetWindowMinimumSize(window, 960, 640);

        sdlRenderer = SDL_CreateRenderer(window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!sdlRenderer) return false;

        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

        font = TTF_OpenFont("assets/DejaVuSans.ttf", 18);
        if (!font) font = TTF_OpenFont("/assets/DejaVuSans.ttf", 18);
        fontSmall = TTF_OpenFont("assets/DejaVuSans.ttf", 14);
        if (!fontSmall) fontSmall = TTF_OpenFont("/assets/DejaVuSans.ttf", 14);
        fontMono = TTF_OpenFont("assets/DejaVuSansMono.ttf", 16);
        if (!fontMono) fontMono = TTF_OpenFont("/assets/DejaVuSansMono.ttf", 16);
        fontTitle = TTF_OpenFont("assets/DejaVuSans.ttf", 36);
        if (!fontTitle) fontTitle = TTF_OpenFont("/assets/DejaVuSans.ttf", 36);

        if (!font || !fontSmall) {
            SDL_Log("Warning: Could not load all fonts. Text rendering may be limited.");
        }

        return true;
    }

    ~Renderer() {
        if (fontTitle) TTF_CloseFont(fontTitle);
        if (fontMono) TTF_CloseFont(fontMono);
        if (fontSmall) TTF_CloseFont(fontSmall);
        if (font) TTF_CloseFont(font);
        if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }

    void setColor(const UIColor& c) {
        SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    }

    void fillRect(const UIRect& r) {
        SDL_Rect sr = {r.x, r.y, r.w, r.h};
        SDL_RenderFillRect(sdlRenderer, &sr);
    }

    void drawRect(const UIRect& r) {
        SDL_Rect sr = {r.x, r.y, r.w, r.h};
        SDL_RenderDrawRect(sdlRenderer, &sr);
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        SDL_RenderDrawLine(sdlRenderer, x1, y1, x2, y2);
    }

    void drawText(const std::string& text, int x, int y, TTF_Font* f, const UIColor& color, bool center = false) {
        if (!f) return;
        SDL_Color c = {color.r, color.g, color.b, color.a};
        SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), c);
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
        if (!tex) { SDL_FreeSurface(surf); return; }
        SDL_Rect dst = {center ? x - surf->w/2 : x, y, surf->w, surf->h};
        SDL_RenderCopy(sdlRenderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surf);
    }

    int textWidth(const std::string& text, TTF_Font* f) {
        if (!f) return 0;
        int w;
        TTF_SizeText(f, text.c_str(), &w, NULL);
        return w;
    }

    void fillGradient(int x, int y, int w, int h, const UIColor& top, const UIColor& bottom) {
        for (int i = 0; i < h; i++) {
            float t = (float)i / h;
            Uint8 r = (Uint8)(top.r + (bottom.r - top.r) * t);
            Uint8 g = (Uint8)(top.g + (bottom.g - top.g) * t);
            Uint8 b = (Uint8)(top.b + (bottom.b - top.b) * t);
            SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
            SDL_RenderDrawLine(sdlRenderer, x, y + i, x + w, y + i);
        }
    }

    void beginFrame() {
        setColor(COLOR_BG);
        SDL_RenderClear(sdlRenderer);
        SDL_GetWindowSize(window, &screenW, &screenH);
        tick++;
    }

    void endFrame() {
        SDL_RenderPresent(sdlRenderer);
    }

    void drawPlayerAvatar(float x, float y) {
        setColor(COLOR_PLAYER);
        int cx = (int)x, cy = (int)y;
        UIRect head = {cx - 7, cy - 22, 14, 14};
        fillRect(head);
        UIRect body = {cx - 15, cy - 8, 30, 35};
        fillRect(body);
    }

    void drawInteractPrompt(const std::string& prompt, float x, float y) {
        if (prompt.empty()) return;
        int w = textWidth(prompt, fontSmall) + 16;
        UIRect bg = {(int)x - w / 2, (int)y - 45, w, 22};
        setColor({15, 12, 22, 220});
        fillRect(bg);
        setColor(COLOR_ACCENT);
        drawRect(bg);
        drawText(prompt, (int)x, (int)y - 41, fontSmall, COLOR_TEXT_BRIGHT, true);
    }

    // === DRAWING FUNCTIONS ===

    void drawLobbyScene(GameEngine& engine) {
        // Background gradient
        fillGradient(0, 0, screenW, screenH, {15, 12, 22, 255}, {25, 20, 35, 255});

        int bottomY = screenH - 200;

        // Floor
        setColor(COLOR_LOBBY_FLOOR);
        UIRect floor = {0, bottomY, screenW, 200};
        fillRect(floor);

        // Floor pattern lines
        setColor({45, 38, 55, 255});
        for (int x = 0; x < screenW; x += 80) {
            drawLine(x, bottomY, x, screenH);
        }

        // Back wall
        fillGradient(0, 0, screenW, bottomY, {25, 20, 38, 255}, {35, 28, 48, 255});

        // Wall panels
        setColor(COLOR_PANEL_BORDER);
        for (int x = 0; x < screenW; x += 200) {
            UIRect panel = {x + 10, 20, 180, bottomY - 40};
            drawRect(panel);
        }

        // Elevator doors (right side)
        int elevatorX = screenW - 150;
        int elevatorW = 100;
        int elevatorH = bottomY - 60;
        int elevatorY = 40;

        setColor(COLOR_ELEVATOR);
        UIRect elevator = {elevatorX, elevatorY, elevatorW, elevatorH};
        fillRect(elevator);

        setColor({80, 80, 90, 255});
        UIRect elevatorInner = {elevatorX + 10, elevatorY + 10, elevatorW - 20, elevatorH - 20};
        fillRect(elevatorInner);

        // Elevator door line
        setColor(COLOR_ELEVATOR);
        drawLine(elevatorX + elevatorW/2, elevatorY + 10, elevatorX + elevatorW/2, elevatorY + elevatorH - 10);

        // Elevator arrow (animated)
        setColor(COLOR_ACCENT);
        float pulse = 0.5f + 0.5f * sinf(tick * 0.1f);
        int arrowAlpha = (int)(128 + 127 * pulse);
        setColor({180, 150, 90, (Uint8)arrowAlpha});
        UIRect arrow = {elevatorX + elevatorW/2 - 8, elevatorY + elevatorH - 30, 16, 16};
        fillRect(arrow);
        drawText("v", elevatorX + elevatorW/2, elevatorY + elevatorH - 35, fontSmall, COLOR_ACCENT, true);

        // Elevator floor indicator
        int floorNum = (tick / 60) % 10;
        std::string floorStr = (floorNum == 0) ? "G" : std::to_string(floorNum);
        drawText(floorStr, elevatorX + elevatorW/2, elevatorY + 10, fontSmall, COLOR_TEXT_BRIGHT, true);

        // Front desk
        int deskX = layout::deskX();
        int deskY = layout::deskY(screenH);
        int deskW = layout::deskW(screenW);
        int deskH = 80;

        setColor(COLOR_DESK);
        UIRect desk = {deskX, deskY, deskW, deskH};
        fillRect(desk);

        setColor({70, 50, 35, 255});
        UIRect deskTop = {deskX, deskY, deskW, 8};
        fillRect(deskTop);

        setColor({90, 65, 40, 255});
        UIRect deskPanel = {deskX + 20, deskY + 20, deskW - 40, deskH - 30};
        fillRect(deskPanel);

        // Desk items - positioned as fractions of deskW so they stay on the
        // desk instead of drifting off it when the window is resized
        int monitorX = deskX + (int)(deskW * 0.08f);
        float phoneX = layout::phoneX(screenW);
        float keyBoardXf = layout::keyBoardX(screenW);
        int keyBoardX = (int)keyBoardXf;
        int bellX = deskX + (int)(deskW * 0.66f);

        // Computer
        setColor({30, 35, 45, 255});
        UIRect monitor = {monitorX, deskY - 50, 120, 50};
        fillRect(monitor);
        setColor({50, 180, 100, 100});
        UIRect screen = {monitorX + 5, deskY - 45, 110, 40};
        fillRect(screen);
        drawText("HOTEL OS v0.1", monitorX + 10, deskY - 38, fontSmall, COLOR_SUCCESS);

        // Phone
        setColor(COLOR_PHONE);
        UIRect phoneBody = {(int)phoneX, deskY - 15, 40, 15};
        fillRect(phoneBody);
        UIRect phoneReceiver = {(int)phoneX + 5, deskY - 25, 30, 10};
        fillRect(phoneReceiver);

        // Phone ringing indicator
        if (engine.state.phoneRinging && engine.ui.flash) {
            setColor(COLOR_DANGER);
            UIRect phoneRing = {(int)phoneX + 10, deskY - 35, 20, 10};
            fillRect(phoneRing);
            drawText("RINGING!", (int)phoneX + 20, deskY - 50, fontSmall, COLOR_DANGER);
        }

        // Key board
        setColor(COLOR_KEY_BOARD);
        UIRect keyBoard = {keyBoardX, deskY - 80, 200, 80};
        fillRect(keyBoard);

        setColor({60, 50, 40, 255});
        UIRect keyBoardBorder = {keyBoardX - 5, deskY - 85, 210, 90};
        drawRect(keyBoardBorder);

        drawText("ROOM KEYS", keyBoardX + 10, deskY - 75, fontSmall, COLOR_TEXT_DIM);

        // Draw key hooks
        for (int i = 0; i < 10; i++) {
            int kx = keyBoardX + 10 + (i % 5) * 38;
            int ky = deskY - 55 + (i / 5) * 22;
            setColor(COLOR_ACCENT);
            UIRect hook = {kx, ky, 4, 4};
            fillRect(hook);

            int roomNum = i + 1;
            bool occupied = false;
            for (auto& r : engine.state.rooms) {
                if (r.number == roomNum && r.occupied) occupied = true;
            }
            if (occupied) {
                setColor(COLOR_DANGER);
                UIRect keyTag = {kx + 5, ky - 2, 6, 8};
                fillRect(keyTag);
            }
        }

        // Bell on desk
        setColor(COLOR_ACCENT);
        UIRect bell = {bellX, deskY - 20, 20, 20};
        fillRect(bell);

        // Guest waiting area (dots representing guests)
        float guestAreaX = layout::guestAreaX();
        float guestAreaY = layout::guestAreaY(screenH);

        drawText("QUEUE", (int)guestAreaX, (int)guestAreaY - 20, fontSmall, COLOR_TEXT_DIM);

        for (size_t i = 0; i < engine.state.waitingGuests.size() && i < 6; i++) {
            auto& guest = engine.state.waitingGuests[i];
            int gx = (int)layout::guestSlotX(screenW, (int)i);
            int gy = (int)guestAreaY;

            // Silhouette
            setColor(COLOR_GUEST_PH);
            UIRect head = {gx + 8, gy, 14, 14};
            fillRect(head);
            UIRect body = {gx, gy + 15, 30, 35};
            fillRect(body);

            // Mood indicator
            UIColor moodColor = COLOR_TEXT;
            if (guest.mood == GuestMood::ANGRY || guest.mood == GuestMood::FRANTIC) moodColor = COLOR_DANGER;
            else if (guest.mood == GuestMood::TERRIFIED) moodColor = COLOR_WARN;
            else if (guest.mood == GuestMood::CHEERFUL) moodColor = COLOR_SUCCESS;
            else if (guest.mood == GuestMood::CONFUSED || guest.mood == GuestMood::DREAMY) moodColor = COLOR_ACCENT2;

            drawText(guest_system::getMoodEmoji(guest.mood), gx + 5, gy + 52, fontSmall, moodColor);
            drawText(guest.name.substr(0, 10), gx - 5, gy + 68, fontSmall, COLOR_TEXT_DIM, true);

            if (guest.patience < 0.3f) {
                setColor(COLOR_DANGER);
                UIRect warningDot = {gx + 25, gy - 5, 5, 5};
                fillRect(warningDot);
            }

            if (guest.id == engine.ui.selectedGuestId) {
                setColor(COLOR_PLAYER);
                UIRect selectRing = {gx - 3, gy - 3, 36, 56};
                drawRect(selectRing);
            }
        }

        // Staff indicators
        int staffStartX = screenW - 230;
        int staffStartY = screenH - 80;

        drawText("STAFF", staffStartX, staffStartY - 20, fontSmall, COLOR_TEXT_DIM);

        for (size_t i = 0; i < engine.state.staff.size(); i++) {
            auto& staff = engine.state.staff[i];
            int sx = staffStartX + (i % 3) * 75;
            int sy = staffStartY + (i / 3) * 45;

            UIColor staffColor = staff.available ? COLOR_SUCCESS : COLOR_WARN;
            setColor(staffColor);
            UIRect staffDot = {sx, sy, 12, 12};
            fillRect(staffDot);

            drawText(staff.name.substr(0, 5), sx + 16, sy - 2, fontSmall, COLOR_TEXT_DIM);
            drawText(hotel_manager::getStaffName(staff.type), sx + 16, sy + 12, fontSmall, COLOR_TEXT_DIM);

            if (!staff.available) {
                drawText(staff.currentTask + "...", sx + 16, sy + 26, fontSmall, COLOR_WARN);
            }
        }

        // Player avatar + nearby interaction prompt
        NearbyInteraction near = engine.getNearbyInteraction(screenW, screenH);
        drawInteractPrompt(near.prompt, engine.state.playerX, engine.state.playerY);
        drawPlayerAvatar(engine.state.playerX, engine.state.playerY);

        if (engine.ui.selectedGuestId >= 0 && engine.isWaitingGuest(engine.ui.selectedGuestId)) {
            drawText("Selected a guest - head to the elevator to assign a room",
                screenW / 2, screenH - 195, fontSmall, COLOR_HIGHLIGHT, true);
        }

        // Shift timer
        int mins = (int)engine.state.shiftTimer / 60;
        int secs = (int)engine.state.shiftTimer % 60;
        char timerBuf[32];
        snprintf(timerBuf, sizeof(timerBuf), "Shift: %02d:%02d", mins, secs);

        UIColor timerColor = engine.state.shiftTimer < 60.0f ? COLOR_DANGER : COLOR_TEXT;
        drawText(timerBuf, screenW - 160, 15, font, timerColor);

        drawText("Day " + std::to_string(engine.state.dayNumber), screenW - 160, 38, fontSmall, COLOR_TEXT_DIM);

        // Alert count
        int alertCount = 0;
        for (auto& a : engine.state.activeAlerts) {
            if (!a.handled) alertCount++;
        }
        if (alertCount > 0) {
            setColor(COLOR_DANGER);
            drawText("ALERTS: " + std::to_string(alertCount), 20, 15, font, COLOR_DANGER);
        }

        // Status message
        if (engine.ui.statusTimer > 0.0f) {
            int alpha = (int)(255.0f * std::min(1.0f, engine.ui.statusTimer));
            UIColor statusCol = {220, 210, 200, (Uint8)alpha};
            drawText(engine.ui.statusMessage, screenW / 2, screenH - 30, font, statusCol, true);
        }
    }

    void drawDimOverlay() {
        setColor({8, 6, 12, 150});
        UIRect overlay = {0, 0, screenW, screenH};
        fillRect(overlay);
    }

    void drawGuestDialogueBox(GameEngine& engine) {
        drawDimOverlay();

        Guest* guest = engine.getSelectedGuest();
        if (!guest) return; // update() clears the selection/screen when a guest vanishes

        int panelW = 700;
        int panelH = 220;
        int panelX = screenW / 2 - panelW / 2;
        int panelY = screenH - panelH - 40;

        setColor(COLOR_PANEL_BG);
        UIRect panel = {panelX, panelY, panelW, panelH};
        fillRect(panel);
        setColor(COLOR_PANEL_BORDER);
        drawRect(panel);

        drawText(guest->name + "  (" + guest_system::getGuestTypeName(guest->type) + ")",
            panelX + 20, panelY + 15, font, COLOR_TEXT_BRIGHT);
        drawText(guest_system::getMoodEmoji(guest->mood) + " " + guest_system::getMoodName(guest->mood),
            panelX + 20, panelY + 40, fontSmall, COLOR_TEXT);

        drawText("\"" + guest->dialogueLine + "\"", panelX + 20, panelY + 65, font, COLOR_HIGHLIGHT);
        drawText("Request: " + guest->request, panelX + 20, panelY + 100, fontSmall, COLOR_TEXT);

        if (!guest->specialNeed.empty()) {
            drawText("Special: " + guest->specialNeed, panelX + 20, panelY + 118, fontSmall, COLOR_ACCENT2);
        }

        if (guest->checkedIn) {
            drawText("ROOM: " + std::to_string(guest->assignedRoom), panelX + 20, panelY + 145, font, COLOR_SUCCESS);
        } else {
            drawText("Selected - walk to the elevator to assign a room", panelX + 20, panelY + 145, fontSmall, COLOR_ACCENT);
        }

        if (guest->patience < 1.0f) {
            int patBarX = panelX + panelW - 220;
            int patBarY = panelY + 20;
            int patBarW = 180;
            int patBarH = 16;

            setColor(COLOR_BUTTON_BG);
            UIRect patBg = {patBarX, patBarY, patBarW, patBarH};
            fillRect(patBg);

            UIColor patColor = guest->patience > 0.5f ? COLOR_SUCCESS :
                               guest->patience > 0.2f ? COLOR_WARN : COLOR_DANGER;
            setColor(patColor);
            UIRect patBar = {patBarX, patBarY, (int)(patBarW * guest->patience), patBarH};
            fillRect(patBar);
        }

        if (guest->complained) {
            drawText("Has complained! Handle with care.", panelX + 20, panelY + 170, fontSmall, COLOR_DANGER);
        }

        drawText("[E / ESC] Close", panelX + panelW - 140, panelY + panelH - 25, fontSmall, COLOR_TEXT_DIM);
    }

    void drawPhoneDialogueBox(GameEngine& engine) {
        drawDimOverlay();

        int panelW = 600;
        int panelH = 220;
        int panelX = screenW / 2 - panelW / 2;
        int panelY = screenH - panelH - 40;

        setColor({30, 25, 40, 255});
        UIRect panel = {panelX, panelY, panelW, panelH};
        fillRect(panel);
        setColor(COLOR_PANEL_BORDER);
        drawRect(panel);

        drawText("INCOMING CALL", panelX + 20, panelY + 15, font, COLOR_TEXT_BRIGHT);
        drawText("CALLER: " + engine.ui.currentCall.callerName, panelX + 20, panelY + 45, fontSmall, COLOR_HIGHLIGHT);

        if (engine.ui.currentCall.fromRoom > 0) {
            drawText("From Room: " + std::to_string(engine.ui.currentCall.fromRoom), panelX + 20, panelY + 65, fontSmall, COLOR_ACCENT2);
        }
        if (engine.ui.currentCall.urgent) {
            drawText("URGENT", panelX + panelW - 100, panelY + 45, fontSmall, COLOR_DANGER);
        }

        drawText("\"" + engine.ui.currentCall.message + "\"", panelX + 20, panelY + 100, font, COLOR_TEXT);

        if (engine.ui.phoneAnswered) {
            drawText("Call answered. Taking notes...", panelX + 20, panelY + 130, fontSmall, COLOR_SUCCESS);
        }

        drawText("[E / ENTER / ESC] Hang Up", panelX + 20, panelY + panelH - 30, fontSmall, COLOR_TEXT_DIM);
    }

    void drawElevatorMenu(GameEngine& engine) {
        fillGradient(0, 0, screenW, screenH, COLOR_BG, COLOR_PANEL_BG);

        drawText("ELEVATOR", screenW / 2, 60, fontTitle, COLOR_TEXT_BRIGHT, true);
        drawText("[W/S] Choose Floor  [E] Go  [ESC] Cancel", screenW / 2, 105, fontSmall, COLOR_TEXT_DIM, true);

        auto floors = room_system::getAllFloors();
        int panelW = 360;
        int panelX = screenW / 2 - panelW / 2;
        int startY = 160;
        int rowH = 50;

        for (size_t i = 0; i < floors.size(); i++) {
            int ry = startY + (int)i * rowH;
            bool selected = ((int)i == engine.state.elevatorMenuIndex);

            setColor(selected ? COLOR_BUTTON_HOVER : COLOR_BUTTON_BG);
            UIRect row = {panelX, ry, panelW, rowH - 6};
            fillRect(row);
            setColor(COLOR_PANEL_BORDER);
            drawRect(row);

            std::string label = room_system::getRoomFloorName(floors[i]) + " Floor";
            drawText(label, panelX + 20, ry + 12, font, selected ? COLOR_TEXT_BRIGHT : COLOR_TEXT);

            int roomCount = (int)room_system::getRoomsOnFloor(engine.state.rooms, floors[i]).size();
            drawText(std::to_string(roomCount) + " room(s)", panelX + panelW - 100, ry + 14, fontSmall, COLOR_TEXT_DIM);
        }
    }

    void drawHallwayScene(GameEngine& engine) {
        fillGradient(0, 0, screenW, screenH, {18, 15, 25, 255}, {30, 25, 40, 255});

        std::string floorLabel = room_system::getRoomFloorName(engine.state.currentFloor) + " Floor";
        drawText(floorLabel, screenW / 2, 20, fontTitle, COLOR_TEXT_BRIGHT, true);
        drawText("[A/D] Walk  [E] Interact  [ESC] Back to Lobby", screenW / 2, 60, fontSmall, COLOR_TEXT_DIM, true);

        int corridorY = screenH / 2 + 40;

        // Corridor floor line
        setColor(COLOR_PANEL_BORDER);
        drawLine(0, corridorY + 60, screenW, corridorY + 60);

        // Elevator at the left edge
        setColor(COLOR_ELEVATOR);
        UIRect elevatorDoor = {(int)layout::hallwayElevatorX() - 30, corridorY - 70, 60, 130};
        fillRect(elevatorDoor);
        drawText("ELEVATOR", (int)layout::hallwayElevatorX(), corridorY - 90, fontSmall, COLOR_TEXT_DIM, true);

        auto doors = room_system::getRoomsOnFloor(engine.state.rooms, engine.state.currentFloor);
        for (size_t i = 0; i < doors.size(); i++) {
            Room* room = doors[i];
            float doorX = layout::hallwayDoorX(screenW, (int)i, (int)doors.size());

            UIColor doorColor = COLOR_DOOR;
            if (room->occupied) doorColor = {60, 30, 30, 255};
            if (room->rule != RoomRule::NORMAL) doorColor = {45, 35, 60, 255};
            if (room->rule != RoomRule::NORMAL && room->occupied) doorColor = {55, 25, 55, 255};

            setColor(doorColor);
            UIRect door = {(int)doorX - 25, corridorY - 60, 50, 120};
            fillRect(door);
            setColor(COLOR_PANEL_BORDER);
            drawRect(door);

            drawText(room_system::getRuleIcon(room->rule), (int)doorX, corridorY - 55, fontSmall, COLOR_ACCENT, true);
            drawText(room->name, (int)doorX, corridorY - 35, fontSmall,
                room->occupied ? COLOR_DANGER : COLOR_SUCCESS, true);
        }

        NearbyInteraction near = engine.getNearbyInteraction(screenW, screenH);
        drawInteractPrompt(near.prompt, engine.state.hallwayPlayerX, (float)(corridorY + 60));
        drawPlayerAvatar(engine.state.hallwayPlayerX, (float)(corridorY + 60));

        // Passive room-info tooltip when standing near a door
        if (near.type == InteractionType::DOOR) {
            Room* sel = hotel_manager::findRoom(engine.state, near.targetId);
            if (sel) {
                int infoY = screenH - 130;
                int infoX = screenW / 2 - 350;
                setColor(COLOR_PANEL_BG);
                UIRect infoPanel = {infoX, infoY, 700, 100};
                fillRect(infoPanel);
                setColor(COLOR_PANEL_BORDER);
                drawRect(infoPanel);

                drawText(sel->name + " (" + room_system::getRuleName(sel->rule) + ")", infoX + 15, infoY + 8, font, COLOR_TEXT_BRIGHT);
                drawText(room_system::getRuleDescription(sel->rule), infoX + 15, infoY + 32, fontSmall, COLOR_TEXT);
                drawText("Status: " + std::string(sel->occupied ? "Occupied" : "Vacant") + " | Times Used: " + std::to_string(sel->timesUsed),
                    infoX + 15, infoY + 54, fontSmall, COLOR_TEXT_DIM);

                if (engine.ui.selectedGuestId >= 0 && engine.isWaitingGuest(engine.ui.selectedGuestId) && !sel->occupied) {
                    drawText("[E] Assign this room to your selected guest", infoX + 15, infoY + 74, fontSmall, COLOR_SUCCESS);
                }
            }
        }

        if (engine.ui.selectedGuestId >= 0 && engine.isWaitingGuest(engine.ui.selectedGuestId)) {
            Guest* g = engine.getSelectedGuest();
            if (g) {
                drawText("Assigning a room for: " + g->name, screenW / 2, 90, fontSmall, COLOR_HIGHLIGHT, true);
            }
        }
    }

    void drawAlertPopup(GameEngine& engine) {
        if (engine.ui.selectedAlertIndex < 0 || engine.ui.selectedAlertIndex >= (int)engine.state.activeAlerts.size()) {
            engine.returnToLobby();
            return;
        }

        Alert& alert = engine.state.activeAlerts[engine.ui.selectedAlertIndex];

        fillGradient(0, 0, screenW, screenH, COLOR_BG, COLOR_PANEL_BG);

        drawText("=== ALERT ===", screenW/2, 40, fontTitle, COLOR_DANGER, true);

        int panelX = screenW/2 - 300;
        int panelY = 100;

        setColor({40, 20, 20, 255});
        UIRect panel = {panelX, panelY, 600, 350};
        fillRect(panel);
        setColor(COLOR_DANGER);
        drawRect(panel);

        drawText(alert.message, panelX + 30, panelY + 30, font, COLOR_TEXT_BRIGHT);

        if (alert.relatedRoom > 0) {
            drawText("Room: " + std::to_string(alert.relatedRoom), panelX + 30, panelY + 70, fontSmall, COLOR_ACCENT2);
        }

        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "Time to resolve: %.0fs", alert.timeLeft);
        UIColor timeColor = alert.timeLeft < 10.0f ? COLOR_DANGER : COLOR_WARN;
        drawText(timeBuf, panelX + 30, panelY + 95, fontSmall, timeColor);

        // Action buttons
        struct { std::string label; std::string action; int yOff; } actions[] = {
            {"[1] Send Security", "SEND_SECURITY", 0},
            {"[2] Send Maintenance", "SEND_MAINTENANCE", 45},
            {"[3] Send Maid", "SEND_MAID", 90},
            {"[ESC] Ignore / Back", "IGNORE", 135},
        };

        for (auto& act : actions) {
            setColor(COLOR_BUTTON_BG);
            UIRect btn = {panelX + 30, panelY + 140 + act.yOff, 250, 35};
            fillRect(btn);
            setColor(COLOR_PANEL_BORDER);
            drawRect(btn);
            drawText(act.label, panelX + 40, panelY + 148 + act.yOff, fontSmall, COLOR_TEXT);
        }

        drawText("Unhandled alerts: " + std::to_string(
            std::count_if(engine.state.activeAlerts.begin(), engine.state.activeAlerts.end(),
                [](const Alert& a) { return !a.handled; })
        ), panelX + 30, panelY + 320, fontSmall, COLOR_TEXT_DIM);
    }

    void drawEndOfDay(GameEngine& engine) {
        fillGradient(0, 0, screenW, screenH, {10, 8, 20, 255}, {20, 16, 35, 255});

        drawText("SHIFT COMPLETE", screenW/2, 100, fontTitle, COLOR_TEXT_BRIGHT, true);
        drawText("Day " + std::to_string(engine.state.dayNumber) + " Summary", screenW/2, 145, font, COLOR_TEXT, true);

        int panelX = screenW/2 - 200;
        int panelY = 200;

        setColor(COLOR_PANEL_BG);
        UIRect panel = {panelX, panelY, 400, 250};
        fillRect(panel);
        setColor(COLOR_PANEL_BORDER);
        drawRect(panel);

        drawText("Guests Served: " + std::to_string(engine.state.guestsServed), panelX + 30, panelY + 30, font, COLOR_SUCCESS);
        drawText("Complaints Handled: " + std::to_string(engine.state.complaintsHandled), panelX + 30, panelY + 65, font, COLOR_TEXT);
        drawText("Rooms Lost: " + std::to_string(engine.state.roomsLost), panelX + 30, panelY + 100, font, COLOR_DANGER);

        drawText("Shift Score: " + std::to_string(engine.state.lastShiftScore), panelX + 30, panelY + 145, font, COLOR_HIGHLIGHT);
        drawText("Total Score: " + std::to_string(engine.state.playerScore), panelX + 30, panelY + 175, font, COLOR_HIGHLIGHT);

        setColor(COLOR_BUTTON_BG);
        UIRect nextBtn = {panelX + 50, panelY + 210, 300, 40};
        fillRect(nextBtn);
        setColor(COLOR_PANEL_BORDER);
        drawRect(nextBtn);
        drawText("[ENTER] Start Next Shift", panelX + 60, panelY + 218, font, COLOR_TEXT);
    }

    void drawTitleScreen(GameEngine& engine) {
        fillGradient(0, 0, screenW, screenH, {8, 6, 16, 255}, {18, 14, 30, 255});

        float alpha = engine.ui.titleAlpha;
        int a = (int)(alpha * 255);

        // Hotel silhouette
        setColor({30, 25, 45, (Uint8)a});
        UIRect building = {screenW/2 - 120, screenH/2 - 180, 240, 200};
        fillRect(building);

        // Windows
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                bool lit = (row * col + tick / 30) % 7 != 0;
                UIColor winColor = lit ? UIColor{180, 160, 80, (Uint8)(a/2)} : UIColor{25, 20, 35, (Uint8)a};
                setColor(winColor);
                UIRect window = {screenW/2 - 100 + col * 40, screenH/2 - 160 + row * 35, 25, 20};
                fillRect(window);
            }
        }

        // Door
        setColor(COLOR_ACCENT);
        UIRect door = {screenW/2 - 25, screenH/2 - 10, 50, 40};
        fillRect(door);

        // Title
        UIColor titleColor = {220, 200, 160, (Uint8)a};
        drawText("NO VACANCY", screenW/2, screenH/2 - 210, fontTitle, titleColor, true);
        drawText("IN ROOM INFINITY", screenW/2, screenH/2 - 175, fontTitle, titleColor, true);

        drawText("A hotel management game where every room breaks reality in a different way.",
            screenW/2, screenH/2 + 60, fontSmall, COLOR_TEXT_DIM, true);

        if (engine.ui.titleAlpha >= 0.9f) {
            float pulse = 0.5f + 0.5f * sinf(tick * 0.08f);
            int pulseA = (int)(155 + 100 * pulse);
            UIColor enterColor = {200, 180, 100, (Uint8)pulseA};
            drawText("[ ENTER ]  Begin Your Shift", screenW/2, screenH/2 + 120, font, enterColor, true);
        }

        drawText("v0.1 - Made with C++, SDL2, and Emscripten", screenW/2, screenH - 30, fontSmall, COLOR_TEXT_DIM, true);
    }

    void drawEventLog(GameEngine& engine) {
        // Docked in the back-wall area (above the walkable floor line) so it
        // never covers the guest queue, desk, or player movement.
        const int MAX_LINES = 4;
        int logX = 10;
        int logY = 70;
        int logW = 320;
        int logH = 14 + MAX_LINES * 16;

        setColor({15, 12, 22, 180});
        UIRect logPanel = {logX, logY, logW, logH};
        fillRect(logPanel);
        setColor(COLOR_PANEL_BORDER);
        drawRect(logPanel);

        drawText("EVENT LOG", logX + 10, logY - 18, fontSmall, COLOR_TEXT_DIM);

        int startIdx = std::max(0, (int)engine.state.eventLog.size() - MAX_LINES);
        for (int i = startIdx; i < (int)engine.state.eventLog.size(); i++) {
            int lineY = logY + 8 + (i - startIdx) * 16;
            drawText(engine.state.eventLog[i], logX + 10, lineY, fontSmall, COLOR_TEXT_DIM);
        }
    }

    void drawHUD(GameEngine& engine) {
        // Top bar
        setColor({18, 14, 28, 220});
        UIRect topBar = {0, 0, screenW, 60};
        fillRect(topBar);
        setColor(COLOR_PANEL_BORDER);
        UIRect topBarBorder = {0, 60, screenW, 1};
        fillRect(topBarBorder);

        drawText("NO VACANCY IN ROOM INFINITY", 15, 8, font, COLOR_ACCENT);

        // Controls help
        drawText("[WASD] Move | [E] Interact | [1-9] Select Guest | [P] Alert | [TAB] Next Alert",
            15, 35, fontSmall, COLOR_TEXT_DIM);

        // Room counts
        int occupiedCount = 0;
        for (auto& r : engine.state.rooms) {
            if (r.occupied) occupiedCount++;
        }

        char statsBuf[128];
        snprintf(statsBuf, sizeof(statsBuf), "Rooms: %d/%zu | Guests Queued: %zu | Score: %d",
            occupiedCount, engine.state.rooms.size() - 1,
            engine.state.waitingGuests.size(), engine.state.playerScore);
        drawText(statsBuf, screenW - 400, 35, fontSmall, COLOR_TEXT_DIM);
    }

    void render(GameEngine& engine) {
        beginFrame();

        switch (engine.ui.currentScreen) {
            case GameEngine::GameScreen::TITLE:
                drawTitleScreen(engine);
                break;
            case GameEngine::GameScreen::LOBBY:
                drawLobbyScene(engine);
                drawHUD(engine);
                drawEventLog(engine);
                break;
            case GameEngine::GameScreen::GUEST_DETAIL:
                drawLobbyScene(engine);
                drawHUD(engine);
                drawGuestDialogueBox(engine);
                break;
            case GameEngine::GameScreen::PHONE_CALL:
                drawLobbyScene(engine);
                drawHUD(engine);
                drawPhoneDialogueBox(engine);
                break;
            case GameEngine::GameScreen::ELEVATOR_MENU:
                drawElevatorMenu(engine);
                break;
            case GameEngine::GameScreen::HALLWAY:
                drawHallwayScene(engine);
                drawHUD(engine);
                break;
            case GameEngine::GameScreen::ALERT_POPUP:
                drawAlertPopup(engine);
                break;
            case GameEngine::GameScreen::END_OF_DAY:
                drawEndOfDay(engine);
                break;
        }

        endFrame();
    }
};

} // namespace ui_renderer
