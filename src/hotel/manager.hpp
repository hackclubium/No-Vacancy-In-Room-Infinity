#pragma once

#include "game/types.hpp"
#include "hotel/rooms.hpp"
#include "hotel/guests.hpp"
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>

namespace hotel_manager {

inline void initStaff(GameState& state) {
    state.staff = {
        {"Bellamy",  StaffType::BELLHOP,     true},
        {"Mabel",    StaffType::MAID,         true},
        {"Rusty",    StaffType::MAINTENANCE,  true},
        {"Pierre",   StaffType::CHEF,         true},
        {"Grigori",  StaffType::SECURITY,     true},
    };
}

inline std::string getStaffName(StaffType type) {
    switch (type) {
        case StaffType::BELLHOP:     return "Bellhop";
        case StaffType::MAID:        return "Maid";
        case StaffType::MAINTENANCE: return "Maintenance";
        case StaffType::CHEF:        return "Chef";
        case StaffType::SECURITY:    return "Security";
        case StaffType::MANAGER:     return "Manager";
    }
    return "Staff";
}

inline void logEvent(GameState& state, const std::string& msg) {
    state.eventLog.push_back("[" + std::to_string(state.dayNumber) + "] " + msg);
    if (state.eventLog.size() > 100) {
        state.eventLog.erase(state.eventLog.begin());
    }
}

inline Guest* findGuest(GameState& state, int guestId) {
    for (auto& g : state.checkedInGuests) {
        if (g.id == guestId) return &g;
    }
    for (auto& g : state.waitingGuests) {
        if (g.id == guestId) return &g;
    }
    return nullptr;
}

inline Room* findRoom(GameState& state, int roomNumber) {
    for (auto& r : state.rooms) {
        if (r.number == roomNumber) return &r;
    }
    return nullptr;
}

inline Room* findRoomForGuest(GameState& state, int guestId) {
    for (auto& r : state.rooms) {
        if (r.guestId == guestId) return &r;
    }
    return nullptr;
}

inline Staff* findAvailableStaff(GameState& state) {
    for (auto& s : state.staff) {
        if (s.available) return &s;
    }
    return nullptr;
}

inline std::vector<Room*> getAvailableRooms(GameState& state) {
    std::vector<Room*> available;
    for (auto& r : state.rooms) {
        if (!r.occupied && r.number != 0) {
            available.push_back(&r);
        }
    }
    return available;
}

inline bool assignRoom(GameState& state, Guest& guest, int roomNumber) {
    Room* room = findRoom(state, roomNumber);
    if (!room || room->occupied) {
        return false;
    }

    guest.assignedRoom = roomNumber;
    guest.checkedIn = true;
    room->occupied = true;
    room->guestId = guest.id;
    room->timesUsed++;
    room->anomalyLevel += 0.1f;

    std::string ruleName = room_system::getRuleName(room->rule);
    logEvent(state, guest.name + " checked into " + room->name + " (" + ruleName + ")");

    if (roomNumber == 0) {
        logEvent(state, guest.name + " booked Room 0. I hope they're not expecting to actually GO there.");
    }

    if (room->rule == RoomRule::DOES_NOT_EXIST && guest.type == GuestType::GHOST) {
        logEvent(state, "The ghost seems... quite happy with Room 0. It's the only room that matches their state of existence.");
    }
    
    if (guest.specialNeed == "No mirrors, please. And perhaps a north-facing room." && 
        room->rule == RoomRule::MIRRORLESS) {
        logEvent(state, guest.name + " is very pleased with the mirror situation. Perfect rating likely.");
    }
    
    state.guestsServed++;
    return true;
}

inline void processRoomAnomalies(GameState& state, float deltaTime) {
    static std::mt19937 rng(std::random_device{}());
    
    for (auto& room : state.rooms) {
        if (!room.occupied) continue;
        
        Guest* guest = findGuest(state, room.guestId);
        if (!guest) continue;
        
        room.anomalyLevel += deltaTime * 0.05f;
        
        switch (room.rule) {
            case RoomRule::DUPLICATES_OCCUPANT: {
                std::uniform_real_distribution<float> dupChance(0.0f, 1.0f);
                if (dupChance(rng) < deltaTime * 0.02f && room.duplicateIds.empty()) {
                    Guest dup = *guest;
                    dup.id = state.nextGuestId++;
                    dup.name = guest->name + " (?)";
                    dup.isDuplicate = true;
                    dup.duplicateOf = guest->id;
                    dup.mood = GuestMood::CONFUSED;
                    dup.dialogueLine = "I... I think I'm the other " + guest->name + "?";
                    state.checkedInGuests.push_back(dup);
                    room.duplicateIds.push_back(dup.id);
                    logEvent(state, "Room 7 has created a duplicate of " + guest->name + "! They're very confused.");
                    
                    Alert alert;
                    alert.type = AlertType::KEY_DUPLICATION;
                    alert.message = "A duplicate guest has appeared in Room 7!";
                    alert.relatedRoom = 7;
                    alert.relatedGuest = dup.id;
                    alert.timeLeft = 45.0f;
                    state.activeAlerts.push_back(alert);
                }
                break;
            }
            case RoomRule::EATS_BAGGAGE: {
                std::uniform_real_distribution<float> eatChance(0.0f, 1.0f);
                if (guest->hasLuggage && eatChance(rng) < deltaTime * 0.03f) {
                    guest->hasLuggage = false;
                    room.lostItems.push_back(guest->name + "'s luggage");
                    logEvent(state, "Room 42 has consumed " + guest->name + "'s luggage. The room seems pleased.");
                    
                    Alert alert;
                    alert.type = AlertType::ROOM_EATING_SOMEONE;
                    alert.message = guest->name + "'s luggage was eaten by Room 42!";
                    alert.relatedRoom = 42;
                    alert.relatedGuest = guest->id;
                    alert.timeLeft = 40.0f;
                    state.activeAlerts.push_back(alert);
                }
                break;
            }
            case RoomRule::MEMORY_WIPE: {
                std::uniform_real_distribution<float> wipeChance(0.0f, 1.0f);
                if (wipeChance(rng) < deltaTime * 0.01f && !guest->complained) {
                    guest->complained = true;
                    guest->dialogueLine = "I'm sorry, why am I here again? Did I... did I check in?";
                    guest->mood = GuestMood::CONFUSED;
                    logEvent(state, guest->name + " has forgotten why they checked in. Room 33 claims innocence.");
                }
                break;
            }
            case RoomRule::MULTIPLIES_BILL: {
                room.billMultiplier *= (1.0f + deltaTime * 0.1f);
                if (room.billMultiplier > 10.0f) {
                    room.billMultiplier = 10.0f;
                }
                break;
            }
            case RoomRule::AMPLIFIES_SOUND: {
                std::uniform_real_distribution<float> noiseChance(0.0f, 1.0f);
                if (noiseChance(rng) < deltaTime * 0.04f) {
                    Alert alert;
                    alert.type = AlertType::NOISE_COMPLAINT;
                    alert.message = "NOISE COMPLAINT from Room 5! The guest in Room 5 can hear someone BREATHING three floors down!";
                    alert.relatedRoom = 5;
                    alert.timeLeft = 30.0f;
                    state.activeAlerts.push_back(alert);
                }
                break;
            }
            case RoomRule::LOST_AND_FOUND: {
                for (auto& otherRoom : state.rooms) {
                    for (auto& item : otherRoom.lostItems) {
                        if (std::find(room.lostItems.begin(), room.lostItems.end(), item) == room.lostItems.end()) {
                            room.lostItems.push_back(item);
                        }
                    }
                }
                break;
            }
            case RoomRule::BIGGER_INSIDE: {
                std::uniform_real_distribution<float> lostChance(0.0f, 1.0f);
                if (lostChance(rng) < deltaTime * 0.02f && guest->hasLuggage) {
                    room.lostItems.push_back(guest->name + "'s left shoe");
                    logEvent(state, guest->name + " reports losing their way to the bathroom in Room 12. Took 3 hours to find the door.");
                }
                break;
            }
            case RoomRule::SWAPS_IDENTITIES: {
                if (room.number == 23) {
                    Room* room24 = findRoom(state, 24);
                    if (room24 && room24->occupied && room.anomalyLevel > 3.0f) {
                        Guest* g24 = findGuest(state, room24->guestId);
                        if (g24) {
                            std::swap(guest->name, g24->name);
                            logEvent(state, "Rooms 23 and 24 have swapped their guests' identities! The guests haven't noticed yet.");
                            room.anomalyLevel = 0.0f;
                        }
                    }
                }
                break;
            }
            case RoomRule::GRAVITY_SHIFT: {
                std::uniform_real_distribution<float> gravityChance(0.0f, 1.0f);
                if (gravityChance(rng) < deltaTime * 0.03f) {
                    logEvent(state, guest->name + " reports everything in Room 77 is now on the ceiling. Including the bed.");
                }
                break;
            }
            default: break;
        }
    }
    
    for (auto& room : state.rooms) {
        for (auto& item : room.lostItems) {
            Room* lostRoom = findRoom(state, 44);
            if (lostRoom && std::find(lostRoom->lostItems.begin(), lostRoom->lostItems.end(), item) == lostRoom->lostItems.end()) {
                lostRoom->lostItems.push_back(item);
            }
        }
    }
}

inline void handleRoomSideEffects(GameState& state, Room& room, Guest& guest) {
    switch (room.rule) {
        case RoomRule::MEMORY_WIPE: {
            guest.request = "I don't remember why I'm here.";
            guest.mood = GuestMood::CONFUSED;
            logEvent(state, guest.name + " has forgotten their original request.");
            break;
        }
        case RoomRule::REVERSES_AGE: {
            int newAge = std::max(1, std::max(0, 50) - (int)(room.anomalyLevel * 10));
            logEvent(state, guest.name + " is getting noticeably younger. They're about " + std::to_string(newAge) + " now.");
            break;
        }
        case RoomRule::DREAM_ROOM: {
            logEvent(state, guest.name + "'s dream about flying manifested. Room 61 now has a small thunderstorm on the ceiling.");
            break;
        }
        case RoomRule::DOES_NOT_EXIST: {
            logEvent(state, guest.name + " is somehow checked into a room that doesn't exist. The paperwork says they're there.");
            break;
        }
        default: break;
    }
}

inline void checkOut(GameState& state, Room& room) {
    if (room.occupied) {
        Guest* guest = findGuest(state, room.guestId);
        if (guest) {
            logEvent(state, guest->name + " checked out of " + room.name + ".");
            guest->checkedIn = false;
            
            auto it = std::remove_if(state.checkedInGuests.begin(), state.checkedInGuests.end(),
                [guest](const Guest& g) { return g.id == guest->id; });
            state.checkedInGuests.erase(it, state.checkedInGuests.end());
        }
        
        room.occupied = false;
        room.guestId = -1;
        room.duplicateIds.clear();
    }
}

inline std::string getRoomStatusDisplay(const Room& room) {
    std::string ruleIcon = room_system::getRuleIcon(room.rule);
    std::string status = room.occupied ? "[OCCUPIED]" : "[VACANT]";
    std::string floorName = room_system::getRoomFloorName(room.floor);
    return ruleIcon + " " + room.name + " " + status + " Floor: " + floorName;
}

inline std::string getDetailedRoomInfo(const Room& room) {
    std::ostringstream oss;
    oss << room.name << " (" << room_system::getRuleName(room.rule) << ")\n";
    oss << room_system::getRuleDescription(room.rule) << "\n";
    oss << "Floor: " << room_system::getRoomFloorName(room.floor) << "\n";
    oss << "Status: " << (room.occupied ? "Occupied" : "Vacant") << "\n";
    if (room.occupied) {
        oss << "Times used: " << room.timesUsed << "\n";
    }
    if (!room.lostItems.empty()) {
        oss << "Items found inside: " << room.lostItems.size() << "\n";
    }
    if (room.rule == RoomRule::MULTIPLIES_BILL) {
        oss << "Current bill multiplier: " << (int)(room.billMultiplier) << "x\n";
    }
    return oss.str();
}

inline void sendStaffToRoom(GameState& state, int roomNumber, StaffType type) {
    Staff* staff = nullptr;
    for (auto& s : state.staff) {
        if (s.type == type && s.available) {
            staff = &s;
            break;
        }
    }
    if (!staff) {
        logEvent(state, "No available " + getStaffName(type) + "!");
        return;
    }
    
    staff->available = false;
    staff->assignedRoom = roomNumber;
    staff->taskTimer = 15.0f;
    
    std::string taskName;
    switch (type) {
        case StaffType::MAID:        taskName = "Cleaning"; break;
        case StaffType::MAINTENANCE: taskName = "Repairing"; break;
        case StaffType::SECURITY:    taskName = "Investigating"; break;
        case StaffType::BELLHOP:     taskName = "Delivering"; break;
        case StaffType::CHEF:        taskName = "Cooking"; break;
        default: taskName = "Working"; break;
    }
    staff->currentTask = taskName;
    
    logEvent(state, staff->name + " (" + getStaffName(type) + ") sent to Room " + std::to_string(roomNumber) + " for " + taskName + ".");
}

inline void updateStaff(GameState& state, float deltaTime) {
    for (auto& staff : state.staff) {
        if (!staff.available) {
            staff.taskTimer -= deltaTime;
            if (staff.taskTimer <= 0.0f) {
                staff.available = true;
                staff.assignedRoom = -1;
                staff.currentTask = "";
                logEvent(state, staff.name + " (" + getStaffName(staff.type) + ") has finished their task.");
            }
        }
    }
}

inline void updateGuestPatience(GameState& state, float deltaTime) {
    for (size_t i = 0; i < state.waitingGuests.size(); ) {
        Guest& guest = state.waitingGuests[i];
        guest.patience -= deltaTime * 0.01f * state.timeSpeed;

        if (guest.patience < 0.3f && guest.patience > 0.0f && !guest.complained) {
            guest.complained = true;
            guest.mood = GuestMood::ANGRY;
            guest.dialogueLine = "How much longer is this going to take?!";
            logEvent(state, guest.name + " is getting impatient!");
        }

        if (guest.patience <= 0.0f) {
            logEvent(state, guest.name + " stormed out! Room lost.");
            state.roomsLost++;
            state.waitingGuests.erase(state.waitingGuests.begin() + i);
            continue;
        }

        i++;
    }
}

inline bool handleAlert(GameState& state, Alert& alert, const std::string& action) {
    if (alert.handled) return true;
    
    if (action == "IGNORE") {
        logEvent(state, "Alert ignored: " + alert.message);
        alert.handled = true;
        return true;
    }
    
    if (action == "SEND_SECURITY") {
        Staff* staff = findAvailableStaff(state);
        if (staff) {
            sendStaffToRoom(state, alert.relatedRoom, StaffType::SECURITY);
            alert.handled = true;
            state.complaintsHandled++;
            logEvent(state, "Sent security to handle: " + alert.message);
            return true;
        }
        logEvent(state, "No staff available to handle alert!");
        return false;
    }
    
    if (action == "SEND_MAINTENANCE") {
        Staff* staff = findAvailableStaff(state);
        if (staff) {
            sendStaffToRoom(state, alert.relatedRoom, StaffType::MAINTENANCE);
            alert.handled = true;
            state.complaintsHandled++;
            logEvent(state, "Sent maintenance to handle: " + alert.message);
            return true;
        }
        return false;
    }
    
    if (action == "SEND_MAID") {
        Staff* staff = findAvailableStaff(state);
        if (staff) {
            sendStaffToRoom(state, alert.relatedRoom, StaffType::MAID);
            alert.handled = true;
            state.complaintsHandled++;
            logEvent(state, "Sent maid to handle: " + alert.message);
            return true;
        }
        return false;
    }
    
    return false;
}

} // namespace hotel_manager
