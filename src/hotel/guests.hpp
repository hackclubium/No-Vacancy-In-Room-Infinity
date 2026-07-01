#pragma once

#include "game/types.hpp"
#include "rooms.hpp"
#include <vector>
#include <string>
#include <random>
#include <algorithm>

namespace guest_system {

inline std::string getGuestTypeName(GuestType type) {
    switch (type) {
        case GuestType::HUMAN: return "Human";
        case GuestType::VAMPIRE: return "Vampire";
        case GuestType::GHOST: return "Ghost";
        case GuestType::TIME_TRAVELER: return "Time Traveler";
        case GuestType::DOPPELGANGER: return "Doppelganger";
        case GuestType::FAE: return "Fae";
        case GuestType::SHADOW_PERSON: return "Shadow";
        case GuestType::BLOB: return "Blob";
        case GuestType::JUST_REALLY_WEIRD: return "Weird";
        case GuestType::INSPECTOR: return "Inspector";
    }
    return "Unknown";
}

inline std::string getMoodName(GuestMood mood) {
    switch (mood) {
        case GuestMood::CALM: return "Calm";
        case GuestMood::NERVOUS: return "Nervous";
        case GuestMood::ANGRY: return "Angry";
        case GuestMood::CONFUSED: return "Confused";
        case GuestMood::CHEERFUL: return "Cheerful";
        case GuestMood::TERRIFIED: return "Terrified";
        case GuestMood::SUSPICIOUS: return "Suspicious";
        case GuestMood::ENTITLED: return "Entitled";
        case GuestMood::DREAMY: return "Dreamy";
        case GuestMood::FRANTIC: return "Frantic";
    }
    return "Unknown";
}

inline std::string getMoodEmoji(GuestMood mood) {
    switch (mood) {
        case GuestMood::CALM: return ":)";
        case GuestMood::NERVOUS: return "o_o";
        case GuestMood::ANGRY: return ">(";
        case GuestMood::CONFUSED: return "O_o";
        case GuestMood::CHEERFUL: return ":D";
        case GuestMood::TERRIFIED: return "D:";
        case GuestMood::SUSPICIOUS: return "-_-";
        case GuestMood::ENTITLED: return ":|";
        case GuestMood::DREAMY: return "*_*";
        case GuestMood::FRANTIC: return "@_@";
    }
    return "?";
}

inline Guest generateGuest(int id, bool forceInspector = false) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> typeDist(0, 8);
    std::uniform_int_distribution<int> moodDist(0, 9);
    
    Guest g;
    g.id = id;
    
    if (forceInspector) {
        g.type = GuestType::INSPECTOR;
        g.mood = GuestMood::SUSPICIOUS;
    } else {
        g.type = static_cast<GuestType>(typeDist(rng));
        g.mood = static_cast<GuestMood>(moodDist(rng));
    }
    
    const std::vector<std::string> firstNames = {
        "Margaret", "Harold", "Celeste", "Bartholomew", "Edith", "Reginald",
        "Ophelia", "Mortimer", "Beatrice", "Cornelius", "Agatha", "Percival",
        "Eleanor", "Wilfred", "Gertrude", "Sebastian", "Mildred", "Humphrey",
        "Prudence", "Algernon", "Euphemia", "Theodore", "Philomena", "Archibald",
        "Seraphina", "Benedict", "Cordelia", "Ezra", "Isadora", "Lazarus"
    };
    
    const std::vector<std::string> lastNames = {
        "Thistlewick", "Pendragon", "Mumblewort", "Crumpet", "Bothersworth",
        "Featherington", "Drizzle", "Snozzberry", "Quibble", "Tiddlywinks",
        "Bumbershoot", "Fizzlebang", "Nickelby", "Dithers", "Puddifoot",
        "Snodgrass", "Chuzzlewit", "Wobbleton", "Flimflam", "Picklewhip",
        "Dufflebottom", "Crackleby", "Squigglington", "Bumbleton", "Twiddleton"
    };
    
    std::uniform_int_distribution<int> firstDist(0, firstNames.size() - 1);
    std::uniform_int_distribution<int> lastDist(0, lastNames.size() - 1);
    
    g.name = firstNames[firstDist(rng)] + " " + lastNames[lastDist(rng)];
    g.checkedIn = false;
    g.complained = false;
    g.hasLuggage = true;
    
    if (g.type == GuestType::VAMPIRE) {
        g.name = "Count " + g.name;
        g.specialNeed = "No mirrors, please. And perhaps a north-facing room.";
    } else if (g.type == GuestType::GHOST) {
        g.name = "The Late " + g.name;
        g.hasLuggage = false;
        g.specialNeed = "I hope the room has good... atmosphere.";
    } else if (g.type == GuestType::TIME_TRAVELER) {
        g.fromFuture = true;
        g.yearsFromFuture = std::uniform_int_distribution<int>(5, 300)(rng);
        g.specialNeed = "I've already stayed here tomorrow. I'd like the same room.";
    } else if (g.type == GuestType::FAE) {
        g.specialNeed = "No iron furniture, and I'll need the exact name on the booking.";
    } else if (g.type == GuestType::SHADOW_PERSON) {
        g.specialNeed = "A room with very dim lighting. Very dim.";
    } else if (g.type == GuestType::BLOB) {
        g.specialNeed = "Extra towels. Many extra towels.";
    } else if (g.type == GuestType::DOPPELGANGER) {
        g.specialNeed = "I'm looking for... myself. Have you seen me?";
    }
    
    const std::vector<std::string> requests = {
        "I'd like a quiet room with a view.",
        "Just the standard room, nothing fancy.",
        "Do you have anything on a high floor?",
        "I need a room with good air circulation.",
        "Something cozy. I've had a long day.",
        "A room without any... surprises, if possible.",
        "I'd prefer something close to the elevator.",
        "Just give me whatever's available. I'm exhausted.",
        "I need a room with a desk. I have work to do.",
        "A room with a bathtub, please. A deep one.",
        "Something with character. I hate boring rooms.",
        "The cheapest room you have. I'm on a budget.",
        "I was told you have... special rooms here?",
        "A room with a king bed. I won't accept less.",
    };
    
    std::uniform_int_distribution<int> reqDist(0, requests.size() - 1);
    g.request = requests[reqDist(rng)];
    
    g.dialogueLine = "Hello! I have a reservation.";
    
    return g;
}

inline Guest generateInspector() {
    static bool inspectorGenerated = false;
    int id = 999;
    Guest g = generateGuest(id, true);
    g.name = "Inspector Vex";
    g.request = "Just observing. Carry on with your work.";
    g.dialogueLine = "Hotel Inspection Bureau. Don't mind me.";
    g.specialNeed = "I need to see everything.";
    inspectorGenerated = true;
    return g;
}

} // namespace guest_system
