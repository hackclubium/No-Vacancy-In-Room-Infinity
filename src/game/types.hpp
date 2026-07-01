#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

enum class GuestType {
    HUMAN,
    VAMPIRE,
    GHOST,
    TIME_TRAVELER,
    DOPPELGANGER,
    FAE,
    SHADOW_PERSON,
    BLOB,
    JUST_REALLY_WEIRD,
    INSPECTOR
};

enum class GuestMood {
    CALM,
    NERVOUS,
    ANGRY,
    CONFUSED,
    CHEERFUL,
    TERRIFIED,
    SUSPICIOUS,
    ENTITLED,
    DREAMY,
    FRANTIC
};

enum class RoomRule {
    NORMAL,
    BIGGER_INSIDE,        // Room 12: Tardis-style
    OPENS_YESTERDAY,      // Room 31: Time portal to yesterday
    DUPLICATES_OCCUPANT,  // Room 7: Creates a doppelganger
    LOST_AND_FOUND,       // Room 44: Contains every lost object
    DOES_NOT_EXIST,       // Room 0: Cannot be physically occupied
    MIRRORLESS,           // Room 13: No reflective surfaces
    INFINITE_CORRIDOR,    // Room 66: The hallway never ends
    REVERSES_AGE,         // Room 99: Guests get younger
    SWAPS_IDENTITIES,     // Room 23: People swap names/bodies
    EATS_BAGGAGE,         // Room 42: Luggage goes missing
    AMPLIFIES_SOUND,      // Room 5: Every noise is deafeningly loud
    TIMELOOP_HOUR,        // Room 8: Last hour repeats forever
    MEMORY_WIPE,          // Room 33: Guests forget why they came
    GRAVITY_SHIFT,        // Room 77: Gravity points sideways
    DREAM_ROOM,           // Room 61: Falling asleep becomes literal
    MULTIPLIES_BILL       // Room 15: Charges multiply overnight
};

enum class StaffType {
    BELLHOP,
    MAID,
    MAINTENANCE,
    CHEF,
    SECURITY,
    MANAGER
};

enum class AlertType {
    FIRE,
    FLOOD,
    NOISE_COMPLAINT,
    ROOM_EATING_SOMEONE,
    PARADOX_DETECTED,
    INSPECTOR_ARRIVING,
    ELEVATOR_SENTIENT,
    KEY_DUPLICATION,
    TEMPORAL_LOOP,
    GUEST_MISSING,
    NONE
};

struct Staff {
    std::string name;
    StaffType type;
    bool available = true;
    int assignedRoom = -1;
    float taskTimer = 0.0f;
    std::string currentTask;
};

struct Guest {
    int id;
    std::string name;
    GuestType type;
    GuestMood mood;
    int assignedRoom = -1;
    float patience = 1.0f;
    std::string request;
    std::string specialNeed;
    bool checkedIn = false;
    bool complained = false;
    bool isDuplicate = false;
    int duplicateOf = -1;
    bool hasLuggage = true;
    bool fromFuture = false;
    int yearsFromFuture = 0;
    bool fromPast = false;
    int yearsFromPast = 0;
    bool wantsRefund = false;
    std::string dialogueLine;
};

struct Room {
    int number;
    RoomRule rule;
    std::string name;
    bool occupied = false;
    int guestId = -1;
    std::vector<int> duplicateIds;
    float anomalyLevel = 0.0f;
    int timesUsed = 0;
    std::string description;
    int floor = 1;
    bool needsMaintenance = false;
    bool isClean = true;
    std::vector<std::string> lostItems;
    float billMultiplier = 1.0f;
};

struct PhoneCall {
    std::string callerName;
    std::string message;
    int fromRoom = -1;
    bool urgent = false;
    bool answered = false;
};

struct Alert {
    AlertType type;
    std::string message;
    int relatedRoom = -1;
    int relatedGuest = -1;
    float timeLeft = 30.0f;
    bool handled = false;
};

struct GameState {
    std::vector<Room> rooms;
    std::vector<Guest> waitingGuests;
    std::vector<Guest> checkedInGuests;
    std::vector<Staff> staff;
    std::vector<PhoneCall> phoneCalls;
    std::vector<Alert> activeAlerts;
    std::vector<std::string> eventLog;
    int playerScore = 0;
    int dayNumber = 1;
    float shiftTimer = 300.0f;
    float timeSpeed = 1.0f;
    int nextGuestId = 1;
    float guestSpawnTimer = 0.0f;
    float guestSpawnInterval = 15.0f;
    float phoneRingTimer = 0.0f;
    float phoneRingElapsed = 0.0f;
    float alertTimer = 0.0f;
    bool phoneRinging = false;
    bool inspectorPresent = false;
    int inspectorGuestId = -1;
    int guestsServed = 0;
    int complaintsHandled = 0;
    int roomsLost = 0;
    int lastShiftScore = 0;

    // Player movement / spatial navigation
    float playerX = 640.0f;
    float playerY = 600.0f;
    float hallwayPlayerX = 640.0f;
    int currentFloor = -1;      // floor currently shown in the HALLWAY screen, -1 = not in a hallway
    int elevatorMenuIndex = 0;  // selected index into the floor list while ELEVATOR_MENU is open
};
