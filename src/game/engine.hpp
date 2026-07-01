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

namespace game_engine {

enum class GameScreen {
    TITLE,
    LOBBY,
    ROOM_LIST,
    GUEST_DETAIL,
    PHONE_CALL,
    ALERT_POPUP,
    END_OF_DAY
};

struct UIAction {
    std::string label;
    std::string action;
    int targetId = -1;
    bool enabled = true;
};

struct UIState {
    GameScreen currentScreen = GameScreen::TITLE;
    int selectedGuestId = -1;
    int selectedRoomNumber = -1;
    std::string roomNumberInput;
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
        hotel_manager::logEvent(state, "=== SHIFT " + std::to_string(state.dayNumber) + " STARTED ===");
        hotel_manager::logEvent(state, "You are behind the front desk. The lobby is quiet. For now.");
        hotel_manager::logEvent(state, "Room keys hang on the wall. The phone sits silently. The elevator hums.");
        
        ui.statusMessage = "Welcome to your shift. Try not to think about the layout.";
        ui.statusTimer = 6.0f;
        
        state.guestSpawnTimer = 3.0f;
        state.shiftTimer = 300.0f;
        state.phoneRingTimer = 12.0f;
        
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
            state.guestSpawnTimer = state.guestSpawnInterval + std::uniform_real_distribution<float>(-5.0f, 5.0f)(rng);
            
            Guest g = guest_system::generateGuest(state.nextGuestId++);
            g.dialogueLine = dialogue_system::getGreeting(g);
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
            state.alertTimer = std::uniform_real_distribution<float>(20.0f, 45.0f)(rng);
            
            if (!state.checkedInGuests.empty()) {
                std::uniform_int_distribution<int> guestDist(0, state.checkedInGuests.size() - 1);
                int idx = guestDist(rng);
                Guest& guest = state.checkedInGuests[idx];
                
                Alert alert;
                alert.relatedGuest = guest.id;
                alert.relatedRoom = guest.assignedRoom;
                alert.timeLeft = 30.0f;
                
                std::vector<std::string> alertMessages = {
                    guest.name + " is complaining about strange noises from the walls.",
                    guest.name + " reports their room has changed color. Twice.",
                    guest.name + " has lost something important in their room.",
                    guest.name + " needs extra towels delivered immediately.",
                    guest.name + " says there's someone else in their mirror who isn't them.",
                    "GUEST MISSING: " + guest.name + " was last seen entering their room.",
                };
                
                std::uniform_int_distribution<int> msgDist(0, alertMessages.size() - 1);
                alert.message = alertMessages[msgDist(rng)];
                
                if (msgDist(rng) == 5) {
                    alert.type = AlertType::GUEST_MISSING;
                } else {
                    alert.type = AlertType::NOISE_COMPLAINT;
                }
                
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
        state.lastShiftScore = score;
        state.playerScore += score;

        hotel_manager::logEvent(state, "=== SHIFT " + std::to_string(state.dayNumber) + " ENDED ===");
        hotel_manager::logEvent(state, "Guests served: " + std::to_string(state.guestsServed));
        hotel_manager::logEvent(state, "Complaints handled: " + std::to_string(state.complaintsHandled));
        hotel_manager::logEvent(state, "Rooms lost: " + std::to_string(state.roomsLost));

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
            
            guest->dialogueLine = dialogue_system::getResponse(*guest, "ASSIGN_ROOM", room);
            
            auto it = std::find_if(state.waitingGuests.begin(), state.waitingGuests.end(),
                [guest](const Guest& g) { return g.id == guest->id; });
            if (it != state.waitingGuests.end()) {
                state.checkedInGuests.push_back(*it);
                state.waitingGuests.erase(it);
            }
            
            ui.selectedGuestId = -1;
            ui.selectedRoomNumber = -1;
            ui.currentScreen = GameScreen::LOBBY;
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
    
    void selectGuest(int guestId) {
        ui.selectedGuestId = guestId;
        ui.selectedRoomNumber = -1;
        ui.currentScreen = GameScreen::GUEST_DETAIL;
    }
    
    void openRoomList() {
        ui.currentScreen = GameScreen::ROOM_LIST;
        ui.roomNumberInput.clear();
    }

    void returnToLobby() {
        ui.currentScreen = GameScreen::LOBBY;
        ui.selectedGuestId = -1;
        ui.selectedRoomNumber = -1;
        ui.selectedAlertIndex = -1;
        ui.roomNumberInput.clear();
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
};

} // namespace game_engine
