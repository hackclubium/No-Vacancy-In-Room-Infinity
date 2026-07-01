#pragma once

#include "game/types.hpp"
#include "game/dialogue.hpp"
#include "game/phone.hpp"
#include "hotel/rooms.hpp"
#include "hotel/guests.hpp"
#include "hotel/manager.hpp"
#include <string>
#include <random>
#include <algorithm>
#include <cmath>

namespace game_engine {

enum class GameScreen {
    TITLE,
    LOBBY,
    GUEST_DETAIL,    // lobby scene + a guest dialogue box overlay
    PHONE_CALL,      // lobby scene + a phone dialogue box overlay
    ELEVATOR_MENU,   // floor-select menu
    HALLWAY,         // walkable corridor for the selected floor
    ALERT_POPUP,
    END_OF_DAY
};

enum class InteractionType {
    NONE,
    GUEST,
    PHONE,
    ELEVATOR,
    DOOR,
    HALLWAY_ELEVATOR
};

struct NearbyInteraction {
    InteractionType type = InteractionType::NONE;
    int targetId = -1; // guestId for GUEST, room number for DOOR
    std::string prompt;
};

// Shared layout math so the engine's interaction logic and the renderer's
// drawing code always agree on where things are, regardless of window size.
namespace layout {
    inline int deskX() { return 50; }
    inline int deskW(int screenW) { return screenW - 250; }
    inline int bottomY(int screenH) { return screenH - 200; }
    inline int deskY(int screenH) { return bottomY(screenH) - 60; }
    inline int elevatorX(int screenW) { return screenW - 150; }

    inline float phoneX(int screenW) { return deskX() + deskW(screenW) * 0.27f; }
    inline float keyBoardX(int screenW) { return deskX() + deskW(screenW) * 0.42f; }
    inline float guestAreaX() { return 110.0f; } // deskX(50) + 60
    inline float guestAreaY(int screenH) { return (float)(bottomY(screenH) + 30); }
    inline float guestSlotX(int screenW, int index) { return guestAreaX() + index * 80.0f; }
    inline float elevatorInteractX(int screenW) { return (float)(elevatorX(screenW) + 50); }
    inline float interactLineY(int screenH) { return (float)(bottomY(screenH) + 15); }

    inline float hallwayDoorX(int screenW, int index, int count) {
        float startX = 220.0f;
        float endX = (float)screenW - 100.0f;
        if (count <= 1) return (startX + endX) / 2.0f;
        float step = (endX - startX) / (float)(count - 1);
        return startX + step * index;
    }

    inline float hallwayElevatorX() { return 80.0f; }
}

struct UIAction {
    std::string label;
    std::string action;
    int targetId = -1;
    bool enabled = true;
};

struct UIState {
    GameScreen currentScreen = GameScreen::TITLE;
    int selectedGuestId = -1;
    int selectedAlertIndex = -1;
    PhoneCall currentCall;
    bool phoneAnswered = false;
    float flashTimer = 0.0f;
    bool flash = false;
    std::string statusMessage;
    float statusTimer = 0.0f;
    std::string titleText = "NO VACANCY IN ROOM INFINITY";
    std::string subtitleText = "Press ENTER to begin your shift";
    float titleAlpha = 1.0f;
    bool titleFadingIn = true;
};

class GameEngine {
public:
    using GameScreen = ::game_engine::GameScreen;

    GameState state;
    UIState ui;
    std::mt19937 rng;

    GameEngine() : rng(std::random_device{}()) {}

    void init() {
        state.rooms = room_system::generateRooms();
        hotel_manager::initStaff(state);

        ui.statusMessage = "Welcome to your shift. Try not to think about the layout.";
        ui.statusTimer = 6.0f;

        state.guestSpawnTimer = 3.0f;
        state.shiftTimer = 300.0f;
        state.phoneRingTimer = std::uniform_real_distribution<float>(20.0f, 25.0f)(rng);
        state.playerX = layout::guestAreaX();
        state.playerY = 600.0f;
        state.currentFloor = -1;
        state.nextInspectorDay = state.dayNumber + std::uniform_int_distribution<int>(3, 5)(rng);

        state.eventLog.clear();
        hotel_manager::logEvent(state, "=== SHIFT " + std::to_string(state.dayNumber) + " STARTED ===");
        hotel_manager::logEvent(state, "You are behind the front desk. The lobby is quiet. For now.");
        hotel_manager::logEvent(state, "Room keys hang on the wall. The phone sits silently. The elevator hums.");
    }

    void update(float deltaTime) {
        if (ui.currentScreen == GameScreen::TITLE) {
            updateTitle(deltaTime);
            return;
        }

        if (ui.statusTimer > 0.0f) {
            ui.statusTimer -= deltaTime;
        }

        ui.flashTimer -= deltaTime;
        if (ui.flashTimer <= 0.0f) {
            ui.flash = !ui.flash;
            ui.flashTimer = 0.5f;
        }

        state.shiftTimer -= deltaTime * state.timeSpeed;

        hotel_manager::updateStaff(state, deltaTime);
        hotel_manager::updateGuestPatience(state, deltaTime);
        hotel_manager::processRoomAnomalies(state, deltaTime);

        if (ui.selectedGuestId >= 0 && !hotel_manager::findGuest(state, ui.selectedGuestId)) {
            ui.selectedGuestId = -1;
            ui.statusMessage = "Your selected guest stormed out!";
            ui.statusTimer = 3.0f;
            if (ui.currentScreen == GameScreen::GUEST_DETAIL) {
                ui.currentScreen = GameScreen::LOBBY;
            }
        }

        spawnGuests(deltaTime);
        triggerPhoneCalls(deltaTime);
        updatePhoneRinging(deltaTime);
        triggerAlerts(deltaTime);
        updateAlerts(deltaTime);

        if (state.shiftTimer <= 0.0f) {
            endShift();
        }
    }

    void updateTitle(float deltaTime) {
        if (ui.titleFadingIn) {
            ui.titleAlpha += deltaTime * 1.5f;
            if (ui.titleAlpha >= 1.0f) {
                ui.titleAlpha = 1.0f;
                ui.titleFadingIn = false;
            }
        }
    }

    void spawnGuests(float deltaTime) {
        state.guestSpawnTimer -= deltaTime;
        if (state.guestSpawnTimer <= 0.0f) {
            state.guestSpawnTimer = state.guestSpawnInterval + std::uniform_real_distribution<float>(-8.0f, 8.0f)(rng);

            Guest g;
            if (!state.inspectorPresent && state.dayNumber >= state.nextInspectorDay) {
                g = guest_system::generateInspector(state.nextGuestId++);
                state.inspectorPresent = true;
                state.inspectorGuestId = g.id;
                state.nextInspectorDay = state.dayNumber + std::uniform_int_distribution<int>(3, 5)(rng);
            } else {
                g = guest_system::generateGuest(state.nextGuestId++);
                g.dialogueLine = dialogue_system::getGreeting(g);
            }
            state.waitingGuests.push_back(g);
            hotel_manager::logEvent(state, g.name + " (" + guest_system::getGuestTypeName(g.type) + ") has arrived at the front desk.");
            ui.statusMessage = g.name + " is waiting at the front desk.";
            ui.statusTimer = 3.0f;
        }
    }

    void triggerPhoneCalls(float deltaTime) {
        if (ui.currentScreen == GameScreen::PHONE_CALL) return;

        state.phoneRingTimer -= deltaTime;
        if (state.phoneRingTimer <= 0.0f && !state.phoneRinging) {
            state.phoneRinging = true;
            state.phoneRingElapsed = 0.0f;
            state.phoneCalls.push_back(phone_system::generatePhoneCall(state));
            hotel_manager::logEvent(state, "The phone is ringing...");
            ui.statusMessage = "RING RING... The phone is ringing!";
            ui.statusTimer = 5.0f;
        }
    }

    void updatePhoneRinging(float deltaTime) {
        if (!state.phoneRinging) return;

        state.phoneRingElapsed += deltaTime;
        if (state.phoneRingElapsed > 20.0f) {
            state.phoneRinging = false;
            state.phoneRingElapsed = 0.0f;
            state.phoneCalls.clear();
            state.phoneRingTimer = std::uniform_real_distribution<float>(10.0f, 20.0f)(rng);
            hotel_manager::logEvent(state, "The phone stopped ringing. Missed call.");
            ui.statusMessage = "You missed a call...";
            ui.statusTimer = 3.0f;
        }
    }

    void triggerAlerts(float deltaTime) {
        state.alertTimer -= deltaTime;
        if (state.alertTimer <= 0.0f) {
            state.alertTimer = std::uniform_real_distribution<float>(40.0f, 70.0f)(rng);

            if (!state.checkedInGuests.empty()) {
                std::uniform_int_distribution<int> guestDist(0, state.checkedInGuests.size() - 1);
                int idx = guestDist(rng);
                Guest& guest = state.checkedInGuests[idx];

                Alert alert;
                alert.relatedGuest = guest.id;
                alert.relatedRoom = guest.assignedRoom;
                alert.timeLeft = 30.0f;

                struct AlertOption { std::string message; AlertType type; };
                std::vector<AlertOption> options = {
                    {guest.name + " is complaining about strange noises from the walls.", AlertType::NOISE_COMPLAINT},
                    {guest.name + " reports their room has changed color. Twice.", AlertType::NOISE_COMPLAINT},
                    {guest.name + " has lost something important in their room.", AlertType::NOISE_COMPLAINT},
                    {guest.name + " needs extra towels delivered immediately.", AlertType::NOISE_COMPLAINT},
                    {guest.name + " says there's someone else in their mirror who isn't them.", AlertType::NOISE_COMPLAINT},
                    {"GUEST MISSING: " + guest.name + " was last seen entering their room.", AlertType::GUEST_MISSING},
                    {guest.name + " says their wastebasket just caught fire. Spontaneously. It's fine now, but still.", AlertType::FIRE},
                    {"Water is seeping under " + guest.name + "'s door from somewhere. There is no bathroom near that wall.", AlertType::FLOOD},
                    {"The elevator just called the front desk directly about " + guest.name + "'s floor. It wants to talk about its feelings.", AlertType::ELEVATOR_SENTIENT},
                };

                std::uniform_int_distribution<int> optDist(0, (int)options.size() - 1);
                const AlertOption& chosen = options[optDist(rng)];
                alert.message = chosen.message;
                alert.type = chosen.type;

                state.activeAlerts.push_back(alert);
                hotel_manager::logEvent(state, "ALERT: " + alert.message);
                ui.statusMessage = "ALERT! " + alert.message;
                ui.statusTimer = 4.0f;
            }
        }
    }

    void updateAlerts(float deltaTime) {
        for (auto& alert : state.activeAlerts) {
            if (!alert.handled) {
                alert.timeLeft -= deltaTime;
                if (alert.timeLeft <= 0.0f) {
                    hotel_manager::logEvent(state, "ALERT EXPIRED: " + alert.message + " — Guest is very unhappy.");
                    state.roomsLost++;
                    alert.handled = true;
                }
            }
        }

        state.activeAlerts.erase(
            std::remove_if(state.activeAlerts.begin(), state.activeAlerts.end(),
                [](const Alert& a) { return a.handled && a.timeLeft <= -5.0f; }),
            state.activeAlerts.end()
        );
    }

    void endShift() {
        int score = state.guestsServed * 10 + state.complaintsHandled * 5 - state.roomsLost * 20;

        bool hadInspector = state.inspectorGuestId >= 0;
        std::string inspectorLine;
        int inspectionResult = 0;
        if (hadInspector) {
            Guest* inspector = hotel_manager::findGuest(state, state.inspectorGuestId);
            if (inspector && inspector->checkedIn) {
                Room* room = hotel_manager::findRoom(state, inspector->assignedRoom);
                int roomScore = room ? room_system::getRoomImpressionScore(room->rule) : 0;
                int waitScore = (int)((inspector->patience - 0.5f) * 40.0f);
                inspectionResult = roomScore + waitScore;
                std::string roomDesc = room ? (room->name + " (" + room_system::getRuleName(room->rule) + ")") : "their room";
                inspectorLine = inspectionResult >= 0
                    ? "Inspector Vex was impressed by " + roomDesc + "."
                    : "Inspector Vex was NOT impressed by " + roomDesc + ".";
            } else {
                inspectionResult = -50;
                inspectorLine = "Inspector Vex left without ever being shown a room. Not a good look.";
            }
            score += inspectionResult;
        }

        state.lastShiftScore = score;
        state.playerScore += score;

        hotel_manager::logEvent(state, "=== SHIFT " + std::to_string(state.dayNumber) + " ENDED ===");
        hotel_manager::logEvent(state, "Guests served: " + std::to_string(state.guestsServed));
        hotel_manager::logEvent(state, "Complaints handled: " + std::to_string(state.complaintsHandled));
        hotel_manager::logEvent(state, "Rooms lost: " + std::to_string(state.roomsLost));

        state.hadInspectorLastShift = hadInspector;
        if (hadInspector) {
            hotel_manager::logEvent(state, inspectorLine);
            hotel_manager::logEvent(state, "Inspection result: " + std::string(inspectionResult >= 0 ? "+" : "") + std::to_string(inspectionResult));
            state.lastInspectorLine = inspectorLine;
            state.lastInspectionResult = inspectionResult;
            state.inspectorGuestId = -1;
            state.inspectorPresent = false;
        }

        ui.currentScreen = GameScreen::END_OF_DAY;
    }

    void nextDay() {
        state.dayNumber++;
        state.shiftTimer = 300.0f;
        state.guestsServed = 0;
        state.complaintsHandled = 0;
        state.roomsLost = 0;

        for (auto& room : state.rooms) {
            if (room.occupied) {
                room.anomalyLevel += 0.5f;
                hotel_manager::checkOut(state, room);
            }
        }

        for (auto& staff : state.staff) {
            staff.available = true;
            staff.assignedRoom = -1;
            staff.taskTimer = 0.0f;
            staff.currentTask = "";
        }

        state.activeAlerts.clear();
        state.currentFloor = -1;

        ui.currentScreen = GameScreen::LOBBY;
        hotel_manager::logEvent(state, "=== SHIFT " + std::to_string(state.dayNumber) + " STARTED ===");
        hotel_manager::logEvent(state, "The hotel seems slightly more... wrong than yesterday.");
    }

    void answerPhone() {
        if (!state.phoneRinging || state.phoneCalls.empty()) return;

        PhoneCall call = state.phoneCalls.back();
        call.answered = true;
        state.phoneCalls.clear();
        state.phoneRinging = false;
        state.phoneRingElapsed = 0.0f;
        state.phoneRingTimer = std::uniform_real_distribution<float>(30.0f, 60.0f)(rng);
        ui.currentCall = call;
        ui.phoneAnswered = true;
        ui.currentScreen = GameScreen::PHONE_CALL;
        hotel_manager::logEvent(state, "Answered call from: " + call.callerName);
    }

    void hangUpPhone() {
        ui.currentScreen = GameScreen::LOBBY;
        ui.phoneAnswered = false;
    }

    void assignRoomToGuest(int roomNumber) {
        if (ui.selectedGuestId < 0) return;

        Guest* guest = hotel_manager::findGuest(state, ui.selectedGuestId);
        if (!guest) return;

        if (hotel_manager::assignRoom(state, *guest, roomNumber)) {
            Room* room = hotel_manager::findRoom(state, roomNumber);
            std::string reaction = dialogue_system::getRoomReaction(*guest, room ? *room : state.rooms[0]);
            hotel_manager::logEvent(state, reaction);

            if (room) {
                hotel_manager::handleRoomSideEffects(state, *room, *guest);
            }

            guest->dialogueLine = dialogue_system::getResponse(*guest, "ASSIGN_ROOM", room);

            auto it = std::find_if(state.waitingGuests.begin(), state.waitingGuests.end(),
                [guest](const Guest& g) { return g.id == guest->id; });
            if (it != state.waitingGuests.end()) {
                state.checkedInGuests.push_back(*it);
                state.waitingGuests.erase(it);
            }

            ui.selectedGuestId = -1;
            ui.statusMessage = guest->dialogueLine;
            ui.statusTimer = 4.0f;
        }
    }

    void handleAlertAction(int alertIndex, const std::string& action) {
        if (alertIndex < 0 || alertIndex >= (int)state.activeAlerts.size()) return;

        hotel_manager::handleAlert(state, state.activeAlerts[alertIndex], action);
        ui.currentScreen = GameScreen::LOBBY;
        ui.selectedAlertIndex = -1;
    }

    void returnToLobby() {
        ui.currentScreen = GameScreen::LOBBY;
        ui.selectedGuestId = -1;
        ui.selectedAlertIndex = -1;
        state.currentFloor = -1;
    }

    void startGame() {
        init();
        ui.currentScreen = GameScreen::LOBBY;
    }

    bool isWaitingGuest(int guestId) {
        for (auto& g : state.waitingGuests) {
            if (g.id == guestId) return true;
        }
        return false;
    }

    Guest* getSelectedGuest() {
        if (ui.selectedGuestId < 0) return nullptr;
        return hotel_manager::findGuest(state, ui.selectedGuestId);
    }

    bool hasUnhandledAlerts() {
        for (auto& a : state.activeAlerts) {
            if (!a.handled) return true;
        }
        return false;
    }

    int getFirstUnhandledAlert() {
        for (int i = 0; i < (int)state.activeAlerts.size(); i++) {
            if (!state.activeAlerts[i].handled) return i;
        }
        return -1;
    }

    // === Movement & spatial interaction ===

    void movePlayer(float dx, float dy, float deltaTime, int screenW, int screenH) {
        if (ui.currentScreen != GameScreen::LOBBY && ui.currentScreen != GameScreen::HALLWAY) return;

        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.0001f) {
            dx /= len;
            dy /= len;
        }
        const float speed = 220.0f;

        if (ui.currentScreen == GameScreen::LOBBY) {
            state.playerX += dx * speed * deltaTime;
            state.playerY += dy * speed * deltaTime;
            float minX = 30.0f;
            float maxX = (float)screenW - 30.0f;
            float minY = (float)layout::bottomY(screenH) + 10.0f;
            float maxY = (float)screenH - 40.0f;
            state.playerX = std::clamp(state.playerX, minX, maxX);
            state.playerY = std::clamp(state.playerY, minY, maxY);
        } else {
            state.hallwayPlayerX += dx * speed * deltaTime;
            state.hallwayPlayerX = std::clamp(state.hallwayPlayerX, 60.0f, (float)screenW - 60.0f);
        }
    }

    NearbyInteraction getNearbyInteraction(int screenW, int screenH) {
        NearbyInteraction result;
        // Kept small enough that adjacent guest slots (80px apart) and the
        // most crowded hallway (11 doors, ~96px apart) never overlap.
        const float RADIUS = 38.0f;

        if (ui.currentScreen == GameScreen::LOBBY) {
            for (size_t i = 0; i < state.waitingGuests.size() && i < 6; i++) {
                float gx = layout::guestSlotX(screenW, (int)i) + 15.0f;
                float gy = layout::guestAreaY(screenH) + 25.0f;
                float dx = state.playerX - gx;
                float dy = state.playerY - gy;
                if (dx * dx + dy * dy < RADIUS * RADIUS) {
                    result.type = InteractionType::GUEST;
                    result.targetId = state.waitingGuests[i].id;
                    result.prompt = "[E] Talk to " + state.waitingGuests[i].name;
                    return result;
                }
            }

            if (state.phoneRinging) {
                float px = layout::phoneX(screenW) + 20.0f;
                float py = layout::interactLineY(screenH);
                float dx = state.playerX - px;
                float dy = state.playerY - py;
                if (dx * dx + dy * dy < RADIUS * RADIUS) {
                    result.type = InteractionType::PHONE;
                    result.prompt = "[E] Answer Phone";
                    return result;
                }
            }

            {
                float ex = layout::elevatorInteractX(screenW);
                float ey = layout::interactLineY(screenH);
                float dx = state.playerX - ex;
                float dy = state.playerY - ey;
                if (dx * dx + dy * dy < RADIUS * RADIUS) {
                    result.type = InteractionType::ELEVATOR;
                    result.prompt = "[E] Call Elevator";
                    return result;
                }
            }
        } else if (ui.currentScreen == GameScreen::HALLWAY) {
            auto doors = room_system::getRoomsOnFloor(state.rooms, state.currentFloor);
            for (size_t i = 0; i < doors.size(); i++) {
                float doorX = layout::hallwayDoorX(screenW, (int)i, (int)doors.size());
                if (std::fabs(state.hallwayPlayerX - doorX) < RADIUS) {
                    result.type = InteractionType::DOOR;
                    result.targetId = doors[i]->number;
                    result.prompt = "[E] " + doors[i]->name;
                    return result;
                }
            }

            if (std::fabs(state.hallwayPlayerX - layout::hallwayElevatorX()) < RADIUS) {
                result.type = InteractionType::HALLWAY_ELEVATOR;
                result.prompt = "[E] Elevator";
                return result;
            }
        }

        return result;
    }

    void interact(int screenW, int screenH) {
        if (ui.currentScreen == GameScreen::LOBBY) {
            NearbyInteraction n = getNearbyInteraction(screenW, screenH);
            if (n.type == InteractionType::GUEST) {
                ui.selectedGuestId = n.targetId;
                ui.currentScreen = GameScreen::GUEST_DETAIL;
            } else if (n.type == InteractionType::PHONE) {
                answerPhone();
            } else if (n.type == InteractionType::ELEVATOR) {
                openElevatorMenu();
            }
        } else if (ui.currentScreen == GameScreen::GUEST_DETAIL) {
            ui.currentScreen = GameScreen::LOBBY;
        } else if (ui.currentScreen == GameScreen::PHONE_CALL) {
            hangUpPhone();
        } else if (ui.currentScreen == GameScreen::ELEVATOR_MENU) {
            confirmFloorSelection(screenW);
        } else if (ui.currentScreen == GameScreen::HALLWAY) {
            NearbyInteraction n = getNearbyInteraction(screenW, screenH);
            if (n.type == InteractionType::DOOR) {
                interactWithDoor(n.targetId);
            } else if (n.type == InteractionType::HALLWAY_ELEVATOR) {
                openElevatorMenu();
            }
        }
    }

    void interactWithDoor(int roomNumber) {
        Room* room = hotel_manager::findRoom(state, roomNumber);
        if (!room) return;

        if (ui.selectedGuestId >= 0 && !room->occupied && isWaitingGuest(ui.selectedGuestId)) {
            assignRoomToGuest(roomNumber);
            return;
        }

        if (ui.selectedGuestId < 0 && !room->occupied) {
            if (!room->isClean) {
                hotel_manager::sendStaffToRoom(state, roomNumber, StaffType::MAID);
            } else if (room->needsMaintenance) {
                hotel_manager::sendStaffToRoom(state, roomNumber, StaffType::MAINTENANCE);
            }
        }
        // Otherwise: no-op. Room info is already shown passively via proximity
        // (see renderer's use of getNearbyInteraction for the info tooltip).
    }

    void openElevatorMenu() {
        ui.currentScreen = GameScreen::ELEVATOR_MENU;
        auto floors = room_system::getAllFloors();
        auto it = std::find(floors.begin(), floors.end(), state.currentFloor);
        state.elevatorMenuIndex = (it != floors.end()) ? (int)(it - floors.begin()) : 0;
    }

    void moveElevatorSelection(int delta) {
        int count = (int)room_system::getAllFloors().size();
        state.elevatorMenuIndex = ((state.elevatorMenuIndex + delta) % count + count) % count;
    }

    void confirmFloorSelection(int screenW) {
        auto floors = room_system::getAllFloors();
        state.currentFloor = floors[state.elevatorMenuIndex];
        state.hallwayPlayerX = (float)screenW / 2.0f;
        ui.currentScreen = GameScreen::HALLWAY;
    }

    void exitHallwayToLobby() {
        ui.currentScreen = GameScreen::LOBBY;
        state.currentFloor = -1;
    }
};

} // namespace game_engine
