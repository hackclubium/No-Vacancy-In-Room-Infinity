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
#include <cstdlib>
#include <cmath>

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
const UIColor COLOR_BG         = {16, 13, 24, 255};
const UIColor COLOR_PANEL_BG   = {32, 27, 46, 255};
const UIColor COLOR_PANEL_BORDER = {92, 76, 122, 255};
const UIColor COLOR_TEXT       = {222, 212, 202, 255};
const UIColor COLOR_TEXT_DIM   = {148, 137, 128, 255};
const UIColor COLOR_TEXT_BRIGHT = {255, 250, 238, 255};
const UIColor COLOR_ACCENT     = {214, 174, 98, 255};
const UIColor COLOR_ACCENT2    = {150, 124, 216, 255};
const UIColor COLOR_WARN       = {230, 149, 60, 255};
const UIColor COLOR_DANGER     = {226, 74, 66, 255};
const UIColor COLOR_SUCCESS    = {104, 193, 124, 255};
const UIColor COLOR_HIGHLIGHT  = {226, 200, 122, 255};
const UIColor COLOR_BUTTON_BG  = {50, 44, 68, 255};
const UIColor COLOR_BUTTON_HOVER = {74, 63, 98, 255};
const UIColor COLOR_LOBBY_FLOOR = {38, 30, 50, 255};
const UIColor COLOR_DESK       = {60, 44, 33, 255};
const UIColor COLOR_ELEVATOR   = {54, 54, 66, 255};
const UIColor COLOR_KEY_BOARD  = {49, 38, 27, 255};
const UIColor COLOR_GUEST_PH   = {84, 74, 96, 255};
const UIColor COLOR_PHONE      = {44, 38, 32, 255};
const UIColor COLOR_PLAYER     = {236, 201, 117, 255};
const UIColor COLOR_DOOR       = {76, 60, 49, 255};
const UIColor COLOR_SHADOW     = {4, 3, 8, 100};

inline UIColor getGuestSilhouetteColor(GuestType type) {
    switch (type) {
        case GuestType::VAMPIRE:          return {72, 38, 52, 255};
        case GuestType::GHOST:            return {150, 160, 182, 150};
        case GuestType::TIME_TRAVELER:    return {84, 108, 132, 255};
        case GuestType::DOPPELGANGER:     return {112, 90, 112, 255};
        case GuestType::FAE:              return {88, 132, 100, 255};
        case GuestType::SHADOW_PERSON:    return {24, 21, 30, 255};
        case GuestType::BLOB:             return {66, 132, 92, 255};
        case GuestType::JUST_REALLY_WEIRD:return {142, 100, 152, 255};
        case GuestType::INSPECTOR:        return {96, 96, 108, 255};
        default:                          return COLOR_GUEST_PH;
    }
}

inline UIColor getImpressionColor(RoomRule rule) {
    int score = room_system::getRoomImpressionScore(rule);
    if (score > 0) return COLOR_SUCCESS;
    if (score < 0) return COLOR_DANGER;
    return COLOR_ACCENT;
}

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

    // Juice state: screen shake, particles, transitions, alert flash
    float shakeTimer = 0.0f;
    float shakeIntensity = 0.0f;
    int shakeOffsetXi = 0;
    int shakeOffsetYi = 0;
    float transitionAlpha = 1.0f;
    float alertFlashTimer = 0.0f;
    bool effectsInitialized = false;
    GameEngine::GameScreen prevScreen = GameEngine::GameScreen::TITLE;
    std::vector<bool> prevRoomOccupied;
    size_t prevAlertCount = 0;
    int prevRoomsLost = 0;

    struct Particle {
        float x, y, vx, vy;
        UIColor color;
        float lifetime;
        float maxLifetime;
    };
    std::vector<Particle> particles;

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
        SDL_Rect sr = {r.x + shakeOffsetXi, r.y + shakeOffsetYi, r.w, r.h};
        SDL_RenderFillRect(sdlRenderer, &sr);
    }

    void drawRect(const UIRect& r) {
        SDL_Rect sr = {r.x + shakeOffsetXi, r.y + shakeOffsetYi, r.w, r.h};
        SDL_RenderDrawRect(sdlRenderer, &sr);
    }

    void drawLine(int x1, int y1, int x2, int y2) {
        SDL_RenderDrawLine(sdlRenderer, x1 + shakeOffsetXi, y1 + shakeOffsetYi, x2 + shakeOffsetXi, y2 + shakeOffsetYi);
    }

    void drawText(const std::string& text, int x, int y, TTF_Font* f, const UIColor& color, bool center = false) {
        if (!f) return;
        SDL_Color c = {color.r, color.g, color.b, color.a};
        SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), c);
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(sdlRenderer, surf);
        if (!tex) { SDL_FreeSurface(surf); return; }
        SDL_Rect dst = {(center ? x - surf->w/2 : x) + shakeOffsetXi, y + shakeOffsetYi, surf->w, surf->h};
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

    std::vector<std::string> wrapText(const std::string& text, TTF_Font* f, int maxWidth) {
        std::vector<std::string> lines;
        std::istringstream iss(text);
        std::string word, currentLine;
        while (iss >> word) {
            std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
            if (textWidth(candidate, f) > maxWidth && !currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                currentLine = candidate;
            }
        }
        if (!currentLine.empty()) lines.push_back(currentLine);
        if (lines.empty()) lines.push_back("");
        return lines;
    }

    // Draws left-aligned wrapped text and returns the total height used.
    int drawTextWrapped(const std::string& text, int x, int y, TTF_Font* f, const UIColor& color, int maxWidth, int lineHeight) {
        auto lines = wrapText(text, f, maxWidth);
        for (size_t i = 0; i < lines.size(); i++) {
            drawText(lines[i], x, y + (int)i * lineHeight, f, color);
        }
        return (int)lines.size() * lineHeight;
    }

    void fillGradient(int x, int y, int w, int h, const UIColor& top, const UIColor& bottom) {
        for (int i = 0; i < h; i++) {
            float t = (float)i / h;
            Uint8 r = (Uint8)(top.r + (bottom.r - top.r) * t);
            Uint8 g = (Uint8)(top.g + (bottom.g - top.g) * t);
            Uint8 b = (Uint8)(top.b + (bottom.b - top.b) * t);
            SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
            SDL_RenderDrawLine(sdlRenderer, x + shakeOffsetXi, y + i + shakeOffsetYi, x + w + shakeOffsetXi, y + i + shakeOffsetYi);
        }
    }

    // === Shapes ===

    void fillCircle(int cx, int cy, int radius, const UIColor& color) {
        setColor(color);
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)std::sqrt((double)std::max(0, radius * radius - dy * dy));
            drawLine(cx - dx, cy + dy, cx + dx, cy + dy);
        }
    }

    void fillRoundedRect(const UIRect& r, int radius, const UIColor& color) {
        if (radius <= 0 || r.w <= 0 || r.h <= 0) { setColor(color); fillRect(r); return; }
        radius = std::min(radius, std::min(r.w, r.h) / 2);
        setColor(color);
        fillRect({r.x + radius, r.y, r.w - 2 * radius, r.h});
        fillRect({r.x, r.y + radius, radius, r.h - 2 * radius});
        fillRect({r.x + r.w - radius, r.y + radius, radius, r.h - 2 * radius});
        fillCircle(r.x + radius, r.y + radius, radius, color);
        fillCircle(r.x + r.w - radius - 1, r.y + radius, radius, color);
        fillCircle(r.x + radius, r.y + r.h - radius - 1, radius, color);
        fillCircle(r.x + r.w - radius - 1, r.y + r.h - radius - 1, radius, color);
    }

    void drawPanelShadow(const UIRect& r, int radius = 10, int offset = 6) {
        fillRoundedRect({r.x + offset, r.y + offset, r.w, r.h}, radius, COLOR_SHADOW);
    }

    // A rounded panel with a shadow behind it - the standard panel look used everywhere.
    void drawPanel(const UIRect& r, const UIColor& bg, const UIColor& border, int radius = 12) {
        drawPanelShadow(r, radius);
        fillRoundedRect(r, radius, bg);
        setColor(border);
        drawRect(r);
    }

    float bob(float phase, float speed, float amplitude) {
        return sinf((float)tick * speed + phase) * amplitude;
    }

    // === Juice: shake / particles / transitions ===

    void triggerShake(float intensity, float duration) {
        shakeIntensity = intensity;
        shakeTimer = duration;
    }

    void spawnBurst(float x, float y, UIColor color, int count = 16) {
        for (int i = 0; i < count; i++) {
            float angle = (float)(rand() % 360) * 0.0174533f;
            float speed = 40.0f + (float)(rand() % 70);
            Particle p;
            p.x = x; p.y = y;
            p.vx = cosf(angle) * speed;
            p.vy = sinf(angle) * speed - 20.0f;
            p.color = color;
            p.lifetime = 0.4f + (float)(rand() % 30) / 100.0f;
            p.maxLifetime = p.lifetime;
            particles.push_back(p);
        }
    }

    void updateEffects() {
        const float dt = 0.016f;

        if (shakeTimer > 0.0f) {
            shakeTimer -= dt;
            if (shakeTimer > 0.0f) {
                int amt = std::max(1, (int)shakeIntensity);
                shakeOffsetXi = (rand() % (amt * 2 + 1)) - amt;
                shakeOffsetYi = (rand() % (amt * 2 + 1)) - amt;
            } else {
                shakeOffsetXi = 0;
                shakeOffsetYi = 0;
            }
        }

        for (auto& p : particles) {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.vy += 90.0f * dt;
            p.lifetime -= dt;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.lifetime <= 0.0f; }), particles.end());

        if (transitionAlpha > 0.0f) {
            transitionAlpha -= dt / 0.25f;
            if (transitionAlpha < 0.0f) transitionAlpha = 0.0f;
        }

        if (alertFlashTimer > 0.0f) {
            alertFlashTimer -= dt;
        }
    }

    void drawParticles() {
        for (auto& p : particles) {
            float t = std::max(0.0f, p.lifetime / p.maxLifetime);
            UIColor c = p.color;
            c.a = (Uint8)(220 * t);
            setColor(c);
            int size = 3;
            SDL_Rect sr = {(int)p.x - size / 2, (int)p.y - size / 2, size, size};
            SDL_RenderFillRect(sdlRenderer, &sr);
        }
    }

    void drawTransitionOverlay() {
        if (transitionAlpha <= 0.0f) return;
        setColor({6, 5, 10, (Uint8)(transitionAlpha * 255)});
        SDL_Rect sr = {0, 0, screenW, screenH};
        SDL_RenderFillRect(sdlRenderer, &sr);
    }

    void drawAlertFlash() {
        if (alertFlashTimer <= 0.0f) return;
        float t = std::min(1.0f, alertFlashTimer / 0.3f);
        setColor({200, 40, 30, (Uint8)(55 * t)});
        SDL_Rect sr = {0, 0, screenW, screenH};
        SDL_RenderFillRect(sdlRenderer, &sr);
    }

    // GUEST_DETAIL/PHONE_CALL are lightweight overlays drawn on top of the
    // lobby, not real screen switches - treat them as LOBBY so opening or
    // closing a dialogue doesn't trigger a jarring fade-to-black every time.
    GameEngine::GameScreen normalizeForTransition(GameEngine::GameScreen s) {
        if (s == GameEngine::GameScreen::GUEST_DETAIL || s == GameEngine::GameScreen::PHONE_CALL) {
            return GameEngine::GameScreen::LOBBY;
        }
        return s;
    }

    void updateScreenTransition(GameEngine::GameScreen current) {
        if (!effectsInitialized) return; // handled by detectVisualEvents' first-frame init
        GameEngine::GameScreen normalized = normalizeForTransition(current);
        if (normalized != prevScreen) {
            transitionAlpha = 1.0f;
            prevScreen = normalized;
        }
    }

    // Frame-to-frame diffing on plain game state (no engine->renderer coupling
    // needed) to trigger juice for events the engine doesn't know about visuals.
    void detectVisualEvents(GameEngine& engine) {
        if (!effectsInitialized) {
            prevRoomOccupied.assign(engine.state.rooms.size(), false);
            for (size_t i = 0; i < engine.state.rooms.size(); i++) {
                prevRoomOccupied[i] = engine.state.rooms[i].occupied;
            }
            prevAlertCount = engine.state.activeAlerts.size();
            prevRoomsLost = engine.state.roomsLost;
            prevScreen = normalizeForTransition(engine.ui.currentScreen);
            effectsInitialized = true;
            return;
        }

        if (prevRoomOccupied.size() == engine.state.rooms.size()) {
            for (size_t i = 0; i < engine.state.rooms.size(); i++) {
                bool nowOcc = engine.state.rooms[i].occupied;
                if (nowOcc && !prevRoomOccupied[i] &&
                    engine.ui.currentScreen == GameEngine::GameScreen::HALLWAY &&
                    engine.state.rooms[i].floor == engine.state.currentFloor) {
                    auto doors = room_system::getRoomsOnFloor(engine.state.rooms, engine.state.currentFloor);
                    for (size_t d = 0; d < doors.size(); d++) {
                        if (doors[d]->number == engine.state.rooms[i].number) {
                            float doorX = layout::hallwayDoorX(screenW, (int)d, (int)doors.size());
                            int corridorY = screenH / 2 + 40;
                            spawnBurst(doorX, (float)corridorY, COLOR_SUCCESS, 18);
                            break;
                        }
                    }
                }
                prevRoomOccupied[i] = nowOcc;
            }
        }

        if (engine.state.activeAlerts.size() > prevAlertCount) {
            triggerShake(4.0f, 0.25f);
            alertFlashTimer = 0.3f;
        }
        prevAlertCount = engine.state.activeAlerts.size();

        if (engine.state.roomsLost > prevRoomsLost) {
            alertFlashTimer = std::max(alertFlashTimer, 0.25f);
        }
        prevRoomsLost = engine.state.roomsLost;
    }

    void beginFrame() {
        setColor(COLOR_BG);
        SDL_RenderClear(sdlRenderer);
        SDL_GetWindowSize(window, &screenW, &screenH);
        tick++;
        updateEffects();
    }

    void endFrame() {
        SDL_RenderPresent(sdlRenderer);
    }

    void drawPlayerAvatar(float x, float y) {
        float bobY = bob(0.0f, 0.12f, 1.5f);
        setColor(COLOR_PLAYER);
        int cx = (int)x, cy = (int)(y + bobY);
        UIRect head = {cx - 7, cy - 22, 14, 14};
        fillRect(head);
        UIRect body = {cx - 15, cy - 8, 30, 35};
        fillRect(body);
    }

    void drawInteractPrompt(const std::string& prompt, float x, float y) {
        if (prompt.empty()) return;
        int w = textWidth(prompt, fontSmall) + 16;
        UIRect bg = {(int)x - w / 2, (int)y - 45, w, 22};
        drawPanel(bg, {15, 12, 22, 220}, COLOR_ACCENT, 8);
        drawText(prompt, (int)x, (int)y - 41, fontSmall, COLOR_TEXT_BRIGHT, true);
    }

    // A pulsing ring drawn around whatever's currently interactable, at a given screen point.
    void drawHoverGlow(float px, float py) {
        float pulse = 0.5f + 0.5f * sinf((float)tick * 0.15f);
        int radius = 24 + (int)(pulse * 4.0f);
        UIColor glowColor = {236, 201, 117, (Uint8)(60 + 60 * pulse)};
        setColor(glowColor);
        drawRect({(int)px - radius, (int)py - radius, radius * 2, radius * 2});
        drawRect({(int)px - radius + 3, (int)py - radius + 3, radius * 2 - 6, radius * 2 - 6});
    }

    void drawRuleBadge(const std::string& icon, int cx, int cy, UIColor color) {
        int w = textWidth(icon, fontSmall) + 12;
        int h = 18;
        fillRoundedRect({cx - w / 2, cy - h / 2, w, h}, 6, {color.r, color.g, color.b, 55});
        setColor(color);
        drawRect({cx - w / 2, cy - h / 2, w, h});
        drawText(icon, cx, cy - 7, fontSmall, color, true);
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

        drawPanelShadow({elevatorX, elevatorY, elevatorW, elevatorH}, 8, 5);
        fillRoundedRect({elevatorX, elevatorY, elevatorW, elevatorH}, 8, COLOR_ELEVATOR);

        fillRoundedRect({elevatorX + 10, elevatorY + 10, elevatorW - 20, elevatorH - 20}, 4, {80, 80, 90, 255});

        // Elevator door line
        setColor(COLOR_ELEVATOR);
        drawLine(elevatorX + elevatorW/2, elevatorY + 10, elevatorX + elevatorW/2, elevatorY + elevatorH - 10);

        // Elevator arrow (animated)
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

        drawPanelShadow({deskX, deskY, deskW, deskH}, 6, 5);
        fillRoundedRect({deskX, deskY, deskW, deskH}, 6, COLOR_DESK);

        setColor({70, 50, 35, 255});
        UIRect deskTop = {deskX, deskY, deskW, 8};
        fillRect(deskTop);

        fillRoundedRect({deskX + 20, deskY + 20, deskW - 40, deskH - 30}, 4, {90, 65, 40, 255});

        // Desk items - positioned as fractions of deskW so they stay on the
        // desk instead of drifting off it when the window is resized
        int monitorX = deskX + (int)(deskW * 0.08f);
        float phoneX = layout::phoneX(screenW);
        float keyBoardXf = layout::keyBoardX(screenW);
        int keyBoardX = (int)keyBoardXf;
        int bellX = deskX + (int)(deskW * 0.66f);

        // Computer
        fillRoundedRect({monitorX, deskY - 50, 120, 50}, 5, {30, 35, 45, 255});
        fillRoundedRect({monitorX + 5, deskY - 45, 110, 40}, 3, {50, 180, 100, 100});
        drawText("HOTEL OS v0.1", monitorX + 10, deskY - 38, fontSmall, COLOR_SUCCESS);

        // Phone
        setColor(COLOR_PHONE);
        UIRect phoneBody = {(int)phoneX, deskY - 15, 40, 15};
        fillRoundedRect(phoneBody, 4, COLOR_PHONE);
        UIRect phoneReceiver = {(int)phoneX + 5, deskY - 25, 30, 10};
        fillRoundedRect(phoneReceiver, 4, COLOR_PHONE);

        // Phone ringing indicator
        if (engine.state.phoneRinging && engine.ui.flash) {
            setColor(COLOR_DANGER);
            UIRect phoneRing = {(int)phoneX + 10, deskY - 35, 20, 10};
            fillRect(phoneRing);
            drawText("RINGING!", (int)phoneX + 20, deskY - 50, fontSmall, COLOR_DANGER);
        }

        // Key board
        fillRoundedRect({keyBoardX, deskY - 80, 200, 80}, 5, COLOR_KEY_BOARD);

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
        fillCircle(bellX + 10, deskY - 10, 10, COLOR_ACCENT);

        // Guest waiting area (dots representing guests)
        float guestAreaX = layout::guestAreaX();
        float guestAreaY = layout::guestAreaY(screenH);

        drawText("QUEUE", (int)guestAreaX, (int)guestAreaY - 20, fontSmall, COLOR_TEXT_DIM);

        NearbyInteraction near = engine.getNearbyInteraction(screenW, screenH);

        for (size_t i = 0; i < engine.state.waitingGuests.size() && i < 6; i++) {
            auto& guest = engine.state.waitingGuests[i];
            float guestBob = bob((float)i * 1.7f, 0.05f, 2.0f);
            int gx = (int)layout::guestSlotX(screenW, (int)i);
            int gy = (int)(guestAreaY + guestBob);

            if (near.type == InteractionType::GUEST && near.targetId == guest.id) {
                drawHoverGlow((float)gx + 15.0f, (float)gy + 25.0f);
            }

            // Silhouette (tinted by guest type)
            UIColor silColor = getGuestSilhouetteColor(guest.type);
            setColor(silColor);
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

        if (near.type == InteractionType::PHONE) {
            drawHoverGlow(phoneX + 20.0f, layout::interactLineY(screenH));
        } else if (near.type == InteractionType::ELEVATOR) {
            drawHoverGlow(layout::elevatorInteractX(screenW), layout::interactLineY(screenH));
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
            fillCircle(sx + 6, sy + 6, 6, staffColor);

            drawText(staff.name.substr(0, 5), sx + 16, sy - 2, fontSmall, COLOR_TEXT_DIM);
            drawText(hotel_manager::getStaffName(staff.type), sx + 16, sy + 12, fontSmall, COLOR_TEXT_DIM);

            if (!staff.available) {
                drawText(staff.currentTask + "...", sx + 16, sy + 26, fontSmall, COLOR_WARN);
            }
        }

        // Player avatar + nearby interaction prompt
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
        int panelX = screenW / 2 - panelW / 2;
        int contentW = panelW - 40;

        auto dialogueLines = wrapText("\"" + guest->dialogueLine + "\"", font, contentW);
        auto requestLines = wrapText("Request: " + guest->request, fontSmall, contentW);
        std::vector<std::string> specialLines;
        if (!guest->specialNeed.empty()) {
            specialLines = wrapText("Special: " + guest->specialNeed, fontSmall, contentW);
        }

        int cursorY = 15;
        int nameY = cursorY; cursorY += 25;
        int moodY = cursorY; cursorY += 25;
        int dialogueY = cursorY; cursorY += (int)dialogueLines.size() * 22 + 8;
        int requestY = cursorY; cursorY += (int)requestLines.size() * 16 + 4;
        int specialY = cursorY;
        if (!specialLines.empty()) cursorY += (int)specialLines.size() * 16 + 4;
        int statusY = cursorY; cursorY += 22;
        int complainedY = cursorY;
        if (guest->complained) cursorY += 20;

        int panelH = cursorY + 35;
        int panelY = screenH - panelH - 40;
        if (panelY < 75) panelY = 75; // stay clear of the event log docked up top

        drawPanel({panelX, panelY, panelW, panelH}, COLOR_PANEL_BG, COLOR_PANEL_BORDER);

        drawText(guest->name + "  (" + guest_system::getGuestTypeName(guest->type) + ")",
            panelX + 20, panelY + nameY, font, COLOR_TEXT_BRIGHT);
        drawText(guest_system::getMoodEmoji(guest->mood) + " " + guest_system::getMoodName(guest->mood),
            panelX + 20, panelY + moodY, fontSmall, COLOR_TEXT);

        drawTextWrapped("\"" + guest->dialogueLine + "\"", panelX + 20, panelY + dialogueY, font, COLOR_HIGHLIGHT, contentW, 22);
        drawTextWrapped("Request: " + guest->request, panelX + 20, panelY + requestY, fontSmall, COLOR_TEXT, contentW, 16);

        if (!specialLines.empty()) {
            drawTextWrapped("Special: " + guest->specialNeed, panelX + 20, panelY + specialY, fontSmall, COLOR_ACCENT2, contentW, 16);
        }

        if (guest->checkedIn) {
            drawText("ROOM: " + std::to_string(guest->assignedRoom), panelX + 20, panelY + statusY, font, COLOR_SUCCESS);
        } else {
            drawText("Selected - walk to the elevator to assign a room", panelX + 20, panelY + statusY, fontSmall, COLOR_ACCENT);
        }

        if (guest->patience < 1.0f) {
            int patBarX = panelX + panelW - 220;
            int patBarY = panelY + 20;
            int patBarW = 180;
            int patBarH = 16;

            fillRoundedRect({patBarX, patBarY, patBarW, patBarH}, 4, COLOR_BUTTON_BG);

            UIColor patColor = guest->patience > 0.5f ? COLOR_SUCCESS :
                               guest->patience > 0.2f ? COLOR_WARN : COLOR_DANGER;
            fillRoundedRect({patBarX, patBarY, (int)(patBarW * guest->patience), patBarH}, 4, patColor);
        }

        if (guest->complained) {
            drawText("Has complained! Handle with care.", panelX + 20, panelY + complainedY, fontSmall, COLOR_DANGER);
        }

        drawText("[E / ESC] Close", panelX + panelW - 140, panelY + panelH - 25, fontSmall, COLOR_TEXT_DIM);
    }

    void drawPhoneDialogueBox(GameEngine& engine) {
        drawDimOverlay();

        int panelW = 600;
        int panelX = screenW / 2 - panelW / 2;
        int contentW = panelW - 40;

        auto messageLines = wrapText("\"" + engine.ui.currentCall.message + "\"", font, contentW);

        int cursorY = 15;
        int titleY = cursorY; cursorY += 30;
        int callerY = cursorY; cursorY += 20;
        int fromRoomY = cursorY;
        if (engine.ui.currentCall.fromRoom > 0) cursorY += 20;
        cursorY += 15;
        int messageY = cursorY; cursorY += (int)messageLines.size() * 22 + 10;
        int answeredY = cursorY;
        if (engine.ui.phoneAnswered) cursorY += 20;

        int panelH = cursorY + 40;
        int panelY = screenH - panelH - 40;
        if (panelY < 75) panelY = 75;

        drawPanel({panelX, panelY, panelW, panelH}, {34, 28, 44, 255}, COLOR_PANEL_BORDER);

        drawText("INCOMING CALL", panelX + 20, panelY + titleY, font, COLOR_TEXT_BRIGHT);
        drawText("CALLER: " + engine.ui.currentCall.callerName, panelX + 20, panelY + callerY, fontSmall, COLOR_HIGHLIGHT);

        if (engine.ui.currentCall.fromRoom > 0) {
            drawText("From Room: " + std::to_string(engine.ui.currentCall.fromRoom), panelX + 20, panelY + fromRoomY, fontSmall, COLOR_ACCENT2);
        }
        if (engine.ui.currentCall.urgent) {
            drawText("URGENT", panelX + panelW - 100, panelY + callerY, fontSmall, COLOR_DANGER);
        }

        drawTextWrapped("\"" + engine.ui.currentCall.message + "\"", panelX + 20, panelY + messageY, font, COLOR_TEXT, contentW, 22);

        if (engine.ui.phoneAnswered) {
            drawText("Call answered. Taking notes...", panelX + 20, panelY + answeredY, fontSmall, COLOR_SUCCESS);
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

            UIRect row = {panelX, ry, panelW, rowH - 6};
            if (selected) {
                float pulse = 0.5f + 0.5f * sinf((float)tick * 0.15f);
                UIColor c = COLOR_BUTTON_HOVER;
                c.r = (Uint8)std::min(255.0f, c.r + pulse * 20.0f);
                fillRoundedRect(row, 8, c);
            } else {
                fillRoundedRect(row, 8, COLOR_BUTTON_BG);
            }
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

        NearbyInteraction near = engine.getNearbyInteraction(screenW, screenH);

        // Elevator at the left edge
        if (near.type == InteractionType::HALLWAY_ELEVATOR) {
            drawHoverGlow(layout::hallwayElevatorX(), (float)(corridorY + 60));
        }
        drawPanelShadow({(int)layout::hallwayElevatorX() - 30, corridorY - 70, 60, 130}, 6, 4);
        fillRoundedRect({(int)layout::hallwayElevatorX() - 30, corridorY - 70, 60, 130}, 6, COLOR_ELEVATOR);
        drawText("ELEVATOR", (int)layout::hallwayElevatorX(), corridorY - 90, fontSmall, COLOR_TEXT_DIM, true);

        auto doors = room_system::getRoomsOnFloor(engine.state.rooms, engine.state.currentFloor);
        for (size_t i = 0; i < doors.size(); i++) {
            Room* room = doors[i];
            float doorX = layout::hallwayDoorX(screenW, (int)i, (int)doors.size());

            UIColor doorColor = COLOR_DOOR;
            if (room->occupied) doorColor = {60, 30, 30, 255};
            if (room->rule != RoomRule::NORMAL) doorColor = {45, 35, 60, 255};
            if (room->rule != RoomRule::NORMAL && room->occupied) doorColor = {55, 25, 55, 255};
            if (!room->occupied && (!room->isClean || room->needsMaintenance)) doorColor = {50, 45, 35, 255};

            if (near.type == InteractionType::DOOR && near.targetId == room->number) {
                drawHoverGlow(doorX, (float)(corridorY + 60));
            }

            UIRect door = {(int)doorX - 25, corridorY - 60, 50, 120};
            drawPanelShadow(door, 5, 3);
            fillRoundedRect(door, 5, doorColor);
            setColor(COLOR_PANEL_BORDER);
            drawRect(door);

            // Door handle
            fillCircle((int)doorX + 16, corridorY, 2, COLOR_ACCENT);

            drawRuleBadge(room_system::getRuleIcon(room->rule), (int)doorX, corridorY - 55, getImpressionColor(room->rule));
            drawText(room->name, (int)doorX, corridorY - 35, fontSmall,
                room->occupied ? COLOR_DANGER : COLOR_SUCCESS, true);
        }

        drawInteractPrompt(near.prompt, engine.state.hallwayPlayerX, (float)(corridorY + 60));
        drawPlayerAvatar(engine.state.hallwayPlayerX, (float)(corridorY + 60));

        // Passive room-info tooltip when standing near a door
        if (near.type == InteractionType::DOOR) {
            Room* sel = hotel_manager::findRoom(engine.state, near.targetId);
            if (sel) {
                int infoY = screenH - 130;
                int infoX = screenW / 2 - 350;
                drawPanel({infoX, infoY, 700, 100}, COLOR_PANEL_BG, COLOR_PANEL_BORDER);

                drawText(sel->name + " (" + room_system::getRuleName(sel->rule) + ")", infoX + 15, infoY + 8, font, COLOR_TEXT_BRIGHT);
                drawText(room_system::getRuleDescription(sel->rule), infoX + 15, infoY + 32, fontSmall, COLOR_TEXT);

                std::string statusLine = "Status: " + std::string(sel->occupied ? "Occupied" : "Vacant") + " | Times Used: " + std::to_string(sel->timesUsed);
                if (!sel->isClean) statusLine += " | Needs cleaning";
                if (sel->needsMaintenance) statusLine += " | Needs maintenance";
                drawText(statusLine, infoX + 15, infoY + 54, fontSmall, COLOR_TEXT_DIM);

                if (engine.ui.selectedGuestId >= 0 && engine.isWaitingGuest(engine.ui.selectedGuestId) && !sel->occupied) {
                    drawText("[E] Assign this room to your selected guest", infoX + 15, infoY + 74, fontSmall, COLOR_SUCCESS);
                } else if (engine.ui.selectedGuestId < 0 && !sel->occupied && !sel->isClean) {
                    drawText("[E] Send a maid to clean this room", infoX + 15, infoY + 74, fontSmall, COLOR_WARN);
                } else if (engine.ui.selectedGuestId < 0 && !sel->occupied && sel->needsMaintenance) {
                    drawText("[E] Send maintenance to fix this room", infoX + 15, infoY + 74, fontSmall, COLOR_WARN);
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

        int panelW = 600;
        int contentW = panelW - 60;
        auto messageLines = wrapText(alert.message, font, contentW);
        int messageH = (int)messageLines.size() * 26;

        drawPanel({panelX, panelY, panelW, 350}, {44, 22, 22, 255}, COLOR_DANGER);

        drawTextWrapped(alert.message, panelX + 30, panelY + 30, font, COLOR_TEXT_BRIGHT, contentW, 26);

        int infoY = panelY + 30 + messageH + 10;
        if (alert.relatedRoom > 0) {
            drawText("Room: " + std::to_string(alert.relatedRoom), panelX + 30, infoY, fontSmall, COLOR_ACCENT2);
            infoY += 25;
        }

        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "Time to resolve: %.0fs", alert.timeLeft);
        UIColor timeColor = alert.timeLeft < 10.0f ? COLOR_DANGER : COLOR_WARN;
        drawText(timeBuf, panelX + 30, infoY, fontSmall, timeColor);

        // Action buttons - anchored to the bottom of the panel so a long,
        // wrapped message never runs into them.
        int actionsY = panelY + 350 - 200;
        struct { std::string label; std::string action; int yOff; } actions[] = {
            {"[1] Send Security", "SEND_SECURITY", 0},
            {"[2] Send Maintenance", "SEND_MAINTENANCE", 45},
            {"[3] Send Maid", "SEND_MAID", 90},
            {"[ESC] Ignore / Back", "IGNORE", 135},
        };

        for (auto& act : actions) {
            UIRect btn = {panelX + 30, actionsY + act.yOff, 250, 35};
            fillRoundedRect(btn, 6, COLOR_BUTTON_BG);
            setColor(COLOR_PANEL_BORDER);
            drawRect(btn);
            drawText(act.label, panelX + 40, actionsY + 8 + act.yOff, fontSmall, COLOR_TEXT);
        }

        drawText("Unhandled alerts: " + std::to_string(
            std::count_if(engine.state.activeAlerts.begin(), engine.state.activeAlerts.end(),
                [](const Alert& a) { return !a.handled; })
        ), panelX + 30, actionsY + 180, fontSmall, COLOR_TEXT_DIM);
    }

    void drawEndOfDay(GameEngine& engine) {
        fillGradient(0, 0, screenW, screenH, {10, 8, 20, 255}, {20, 16, 35, 255});

        drawText("SHIFT COMPLETE", screenW/2, 100, fontTitle, COLOR_TEXT_BRIGHT, true);
        drawText("Day " + std::to_string(engine.state.dayNumber) + " Summary", screenW/2, 145, font, COLOR_TEXT, true);

        int panelW = 480;
        int panelX = screenW / 2 - panelW / 2;
        int contentW = panelW - 60;

        std::vector<std::string> inspectorLines;
        if (engine.state.hadInspectorLastShift) {
            inspectorLines = wrapText(engine.state.lastInspectorLine, fontSmall, contentW);
        }

        int cursorY = 30;
        int guestsY = cursorY; cursorY += 35;
        int complaintsY = cursorY; cursorY += 35;
        int lostY = cursorY; cursorY += 45;
        int shiftScoreY = cursorY; cursorY += 30;
        int totalScoreY = cursorY; cursorY += 30;
        int inspectorY = cursorY;
        if (!inspectorLines.empty()) {
            cursorY += (int)inspectorLines.size() * 18 + 4 + 20;
        }
        int buttonY = cursorY + 15;
        int panelH = buttonY + 50;
        int panelY = 200;

        drawPanel({panelX, panelY, panelW, panelH}, COLOR_PANEL_BG, COLOR_PANEL_BORDER);

        drawText("Guests Served: " + std::to_string(engine.state.guestsServed), panelX + 30, panelY + guestsY, font, COLOR_SUCCESS);
        drawText("Complaints Handled: " + std::to_string(engine.state.complaintsHandled), panelX + 30, panelY + complaintsY, font, COLOR_TEXT);
        drawText("Rooms Lost: " + std::to_string(engine.state.roomsLost), panelX + 30, panelY + lostY, font, COLOR_DANGER);

        drawText("Shift Score: " + std::to_string(engine.state.lastShiftScore), panelX + 30, panelY + shiftScoreY, font, COLOR_HIGHLIGHT);
        drawText("Total Score: " + std::to_string(engine.state.playerScore), panelX + 30, panelY + totalScoreY, font, COLOR_HIGHLIGHT);

        if (!inspectorLines.empty()) {
            int y = panelY + inspectorY;
            for (auto& line : inspectorLines) {
                drawText(line, panelX + 30, y, fontSmall, COLOR_ACCENT2);
                y += 18;
            }
            UIColor resultColor = engine.state.lastInspectionResult >= 0 ? COLOR_SUCCESS : COLOR_DANGER;
            std::string resultText = "Inspection result: " +
                std::string(engine.state.lastInspectionResult >= 0 ? "+" : "") + std::to_string(engine.state.lastInspectionResult);
            drawText(resultText, panelX + 30, y + 4, fontSmall, resultColor);
        }

        UIRect nextBtn = {panelX + 50, panelY + buttonY, panelW - 100, 40};
        fillRoundedRect(nextBtn, 8, COLOR_BUTTON_BG);
        setColor(COLOR_PANEL_BORDER);
        drawRect(nextBtn);
        drawText("[ENTER] Start Next Shift", panelX + 60, panelY + buttonY + 8, font, COLOR_TEXT);
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
        int contentW = logW - 20;

        // Walk backward from the newest entry, word-wrapping each one to fit
        // the panel width, until the fixed line budget is used up.
        std::vector<std::vector<std::string>> wrappedEntries;
        int totalLines = 0;
        for (int i = (int)engine.state.eventLog.size() - 1; i >= 0 && totalLines < MAX_LINES; i--) {
            auto lines = wrapText(engine.state.eventLog[i], fontSmall, contentW);
            int remaining = MAX_LINES - totalLines;
            if ((int)lines.size() > remaining) lines.resize(remaining);
            wrappedEntries.push_back(lines);
            totalLines += (int)lines.size();
        }
        std::reverse(wrappedEntries.begin(), wrappedEntries.end());

        drawPanel({logX, logY, logW, logH}, {15, 12, 22, 180}, COLOR_PANEL_BORDER, 6);

        drawText("EVENT LOG", logX + 10, logY - 18, fontSmall, COLOR_TEXT_DIM);

        int lineY = logY + 8;
        for (auto& entryLines : wrappedEntries) {
            for (auto& line : entryLines) {
                drawText(line, logX + 10, lineY, fontSmall, COLOR_TEXT_DIM);
                lineY += 16;
            }
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
        detectVisualEvents(engine);
        updateScreenTransition(engine.ui.currentScreen);

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

        drawParticles();
        drawAlertFlash();
        drawTransitionOverlay();

        endFrame();
    }
};

} // namespace ui_renderer
