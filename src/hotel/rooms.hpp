#pragma once

#include "game/types.hpp"
#include <string>
#include <vector>
#include <random>
#include <algorithm>

namespace room_system {

inline std::string getRuleName(RoomRule rule) {
    switch (rule) {
        case RoomRule::NORMAL: return "Standard";
        case RoomRule::BIGGER_INSIDE: return "Spatial Anomaly";
        case RoomRule::OPENS_YESTERDAY: return "Temporal Shift";
        case RoomRule::DUPLICATES_OCCUPANT: return "Duplication Field";
        case RoomRule::LOST_AND_FOUND: return "Lost & Found";
        case RoomRule::DOES_NOT_EXIST: return "Non-Existent";
        case RoomRule::MIRRORLESS: return "Mirrorless";
        case RoomRule::INFINITE_CORRIDOR: return "Infinite Corridor";
        case RoomRule::REVERSES_AGE: return "Age Reversal";
        case RoomRule::SWAPS_IDENTITIES: return "Identity Swap";
        case RoomRule::EATS_BAGGAGE: return "Luggage Eater";
        case RoomRule::AMPLIFIES_SOUND: return "Amplification";
        case RoomRule::TIMELOOP_HOUR: return "Time Loop";
        case RoomRule::MEMORY_WIPE: return "Memory Wipe";
        case RoomRule::GRAVITY_SHIFT: return "Gravity Shift";
        case RoomRule::DREAM_ROOM: return "Dreamscape";
        case RoomRule::MULTIPLIES_BILL: return "Bill Multiplier";
    }
    return "Unknown";
}

inline std::string getRuleDescription(RoomRule rule) {
    switch (rule) {
        case RoomRule::NORMAL: return "A perfectly normal room. Suspiciously normal.";
        case RoomRule::BIGGER_INSIDE: return "The interior is vastly larger than the exterior. Great for storage, terrible for finding the bathroom.";
        case RoomRule::OPENS_YESTERDAY: return "The door opens to exactly 24 hours ago. Useful for do-overs, confusing for checkout times.";
        case RoomRule::DUPLICATES_OCCUPANT: return "Anyone who sleeps here wakes up with a duplicate. The duplicate is usually the nicer one.";
        case RoomRule::LOST_AND_FOUND: return "Everything lost in the hotel eventually ends up here. Including the things you didn't know were missing.";
        case RoomRule::DOES_NOT_EXIST: return "Room 0 is a bureaucratic ghost. It's on the books, the key exists, but the room itself simply isn't.";
        case RoomRule::MIRRORLESS: return "Any reflective surface brought into this room vanishes. Vampires love it. Narcissists hate it.";
        case RoomRule::INFINITE_CORRIDOR: return "The hallway continues forever. Room service takes approximately... forever.";
        case RoomRule::REVERSES_AGE: return "Guests get younger the longer they stay. Check-out before they're a toddler or they can't sign the bill.";
        case RoomRule::SWAPS_IDENTITIES: return "Guests in adjacent rooms occasionally swap bodies. They rarely notice.";
        case RoomRule::EATS_BAGGAGE: return "Suitcases placed on the floor are slowly absorbed. The room seems... satisfied.";
        case RoomRule::AMPLIFIES_SOUND: return "Every whisper becomes a shout. Every footstep an earthquake. The snoring is legendary.";
        case RoomRule::TIMELOOP_HOUR: return "The same hour repeats endlessly. Great for catching up on sleep, bad for catching flights.";
        case RoomRule::MEMORY_WIPE: return "Guests forget why they entered. Some forget who they are. Room service is never remembered.";
        case RoomRule::GRAVITY_SHIFT: return "Gravity points toward the nearest wall. The furniture is bolted to... well, to something.";
        case RoomRule::DREAM_ROOM: return "Falling asleep here makes dreams physically manifest. Hope they're having nice dreams.";
        case RoomRule::MULTIPLIES_BILL: return "Room charges multiply by 2 every night. The fine print is very, very fine.";
    }
    return "Unknown";
}

inline std::string getRuleIcon(RoomRule rule) {
    switch (rule) {
        case RoomRule::NORMAL: return "[~]";
        case RoomRule::BIGGER_INSIDE: return "[<>]";
        case RoomRule::OPENS_YESTERDAY: return "[<--]";
        case RoomRule::DUPLICATES_OCCUPANT: return "[x2]";
        case RoomRule::LOST_AND_FOUND: return "[?]";
        case RoomRule::DOES_NOT_EXIST: return "[0]";
        case RoomRule::MIRRORLESS: return "[//]";
        case RoomRule::INFINITE_CORRIDOR: return "[...]";
        case RoomRule::REVERSES_AGE: return "[<<]";
        case RoomRule::SWAPS_IDENTITIES: return "[<->]";
        case RoomRule::EATS_BAGGAGE: return "[nom]";
        case RoomRule::AMPLIFIES_SOUND: return "[!!!]";
        case RoomRule::TIMELOOP_HOUR: return "[O]";
        case RoomRule::MEMORY_WIPE: return "[ ]";
        case RoomRule::GRAVITY_SHIFT: return "[/]";
        case RoomRule::DREAM_ROOM: return "[zz]";
        case RoomRule::MULTIPLIES_BILL: return "[$$]";
    }
    return "[?]";
}

inline std::vector<Room> generateRooms() {
    std::vector<Room> rooms;
    
    rooms.push_back({0, RoomRule::DOES_NOT_EXIST, "Room 0", false, -1, {}, 0.0f, 0, 
        "The key exists. The booking exists. The room does not.", 0});
    rooms.push_back({1, RoomRule::NORMAL, "Room 1", false, -1, {}, 0.0f, 0, 
        "A standard room. Almost boring. The lamp may blink in Morse code.", 1});
    rooms.push_back({2, RoomRule::NORMAL, "Room 2", false, -1, {}, 0.0f, 0,
        "Comfortable. The wallpaper pattern seems to change when you're not looking.", 1});
    rooms.push_back({3, RoomRule::NORMAL, "Room 3", false, -1, {}, 0.0f, 0,
        "Cozy. The window shows a city that may not be the one outside.", 1});
    rooms.push_back({4, RoomRule::NORMAL, "Room 4", false, -1, {}, 0.0f, 0,
        "Nice view. The view is always of a different season.", 1});
    rooms.push_back({5, RoomRule::AMPLIFIES_SOUND, "Room 5", false, -1, {}, 0.0f, 0,
        "The quiet room. Every pin drop sounds like thunder.", 1});
    rooms.push_back({6, RoomRule::NORMAL, "Room 6", false, -1, {}, 0.0f, 0,
        "The bed is comfortable. Too comfortable. Guests oversleep by days.", 1});
    rooms.push_back({7, RoomRule::DUPLICATES_OCCUPANT, "Room 7", false, -1, {}, 0.0f, 0,
        "The mirror wall is not actually a mirror.", 1});
    rooms.push_back({8, RoomRule::TIMELOOP_HOUR, "Room 8", false, -1, {}, 0.0f, 0,
        "The clock is stuck at 3:14 AM. It's always been 3:14 AM.", 1});
    rooms.push_back({9, RoomRule::NORMAL, "Room 9", false, -1, {}, 0.0f, 0,
        "The TV only shows channels that don't exist.", 1});
    rooms.push_back({10, RoomRule::NORMAL, "Room 10", false, -1, {}, 0.0f, 0,
        "The minibar restocks itself. Never with what you want.", 1});
    rooms.push_back({11, RoomRule::NORMAL, "Room 11", false, -1, {}, 0.0f, 0,
        "The bathroom mirror shows your reflection 5 seconds in the past.", 1});
    rooms.push_back({12, RoomRule::BIGGER_INSIDE, "Room 12", false, -1, {}, 0.0f, 0,
        "The door is standard size. The room contains approximately 3 square miles.", 2});
    rooms.push_back({13, RoomRule::MIRRORLESS, "Room 13", false, -1, {}, 0.0f, 0,
        "No mirrors. No windows. No reflections of any kind.", 2});
    rooms.push_back({14, RoomRule::NORMAL, "Room 14", false, -1, {}, 0.0f, 0,
        "The air conditioning whispers secrets.", 2});
    rooms.push_back({15, RoomRule::MULTIPLIES_BILL, "Room 15", false, -1, {}, 0.0f, 0,
        "The rate seems reasonable. The second night doubles. The third night...", 2});
    rooms.push_back({16, RoomRule::NORMAL, "Room 16", false, -1, {}, 0.0f, 0,
        "Standard room. The carpet pattern is a map of somewhere that doesn't exist.", 2});
    rooms.push_back({17, RoomRule::NORMAL, "Room 17", false, -1, {}, 0.0f, 0,
        "The paintings' eyes follow you. Not metaphorically — the paintings walk around at night.", 2});
    rooms.push_back({18, RoomRule::NORMAL, "Room 18", false, -1, {}, 0.0f, 0,
        "The phone sometimes rings with calls from numbers that don't exist.", 2});
    rooms.push_back({19, RoomRule::NORMAL, "Room 19", false, -1, {}, 0.0f, 0,
        "The shower water is always exactly room temperature. Room temperature of which room is unclear.", 2});
    rooms.push_back({20, RoomRule::NORMAL, "Room 20", false, -1, {}, 0.0f, 0,
        "The door locks from the inside. And the outside. And sometimes both simultaneously.", 2});
    rooms.push_back({21, RoomRule::NORMAL, "Room 21", false, -1, {}, 0.0f, 0,
        "The room service menu lists dishes that have never existed.", 2});
    rooms.push_back({22, RoomRule::NORMAL, "Room 22", false, -1, {}, 0.0f, 0,
        "The closet leads to a different closet each time you open it.", 3});
    rooms.push_back({23, RoomRule::SWAPS_IDENTITIES, "Room 23", false, -1, {}, 0.0f, 0,
        "Guests here occasionally swap with whoever is in Room 24.", 3});
    rooms.push_back({24, RoomRule::SWAPS_IDENTITIES, "Room 24", false, -1, {}, 0.0f, 0,
        "Guests here occasionally swap with whoever is in Room 23.", 3});
    rooms.push_back({25, RoomRule::NORMAL, "Room 25", false, -1, {}, 0.0f, 0,
        "The bed is never quite the same size twice.", 3});
    rooms.push_back({26, RoomRule::NORMAL, "Room 26", false, -1, {}, 0.0f, 0,
        "All clocks in this room show different times. They're all correct.", 3});
    rooms.push_back({31, RoomRule::OPENS_YESTERDAY, "Room 31", false, -1, {}, 0.0f, 0,
        "The door opens to yesterday. Checkout is... complicated.", 3});
    rooms.push_back({33, RoomRule::MEMORY_WIPE, "Room 33", false, -1, {}, 0.0f, 0,
        "Guests forget why they checked in. Sometimes forget they checked in at all.", 4});
    rooms.push_back({42, RoomRule::EATS_BAGGAGE, "Room 42", false, -1, {}, 0.0f, 0,
        "The answer to life, the universe, and where your suitcase went.", 4});
    rooms.push_back({44, RoomRule::LOST_AND_FOUND, "Room 44", false, -1, {}, 0.0f, 0,
        "Containment zone for everything the hotel has ever misplaced. The piles are... impressive.", 4});
    rooms.push_back({61, RoomRule::DREAM_ROOM, "Room 61", false, -1, {}, 0.0f, 0,
        "Sweet dreams... literally. What you dream becomes briefly real.", 4});
    rooms.push_back({66, RoomRule::INFINITE_CORRIDOR, "Room 66", false, -1, {}, 0.0f, 0,
        "The hallway to Room 66 is still being measured. Current estimate: infinite.", 5});
    rooms.push_back({77, RoomRule::GRAVITY_SHIFT, "Room 77", false, -1, {}, 0.0f, 0,
        "Gravity is a suggestion, not a law. The suggestion is: sideways.", 5});
    rooms.push_back({88, RoomRule::NORMAL, "Room 88", false, -1, {}, 0.0f, 0,
        "The room number is 88. It's never been anything else. Please stop asking about Rooms 85-87.", 5});
    rooms.push_back({99, RoomRule::REVERSES_AGE, "Room 99", false, -1, {}, 0.0f, 0,
        "The penthouse. You check in at 80, you check out at 20. Literally.", 9});
    
    return rooms;
}

inline Room getRoomByNumber(const std::vector<Room>& rooms, int number) {
    for (const auto& r : rooms) {
        if (r.number == number) return r;
    }
    return rooms[0];
}

inline std::string getRoomFloorName(int floor) {
    switch (floor) {
        case 0: return "Ground";
        case 1: return "1st";
        case 2: return "2nd";
        case 3: return "3rd";
        case 4: return "4th";
        case 5: return "5th";
        case 9: return "Penthouse";
        default: return "???";
    }
}

// The elevator only stops at floors that actually have rooms on them.
inline std::vector<int> getAllFloors() {
    return {0, 1, 2, 3, 4, 5, 9};
}

inline std::vector<Room*> getRoomsOnFloor(std::vector<Room>& rooms, int floor) {
    std::vector<Room*> result;
    for (auto& r : rooms) {
        if (r.floor == floor) result.push_back(&r);
    }
    return result;
}

} // namespace room_system
