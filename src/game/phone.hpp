#pragma once

#include "game/types.hpp"
#include "hotel/rooms.hpp"
#include "hotel/guests.hpp"
#include <string>
#include <vector>
#include <random>

namespace phone_system {

inline std::string getRandomCallerName() {
    static std::mt19937 rng(std::random_device{}());
    static const std::vector<std::string> names = {
        "Mrs. Pemberton", "Mr. Snodgrass", "The Entity in Room 12",
        "A very confused pigeon", "Someone from Room 0 (how?)",
        "Management (the real one)", "Maintenance", "Unknown Caller",
        "Captain Thistlewick", "The Minibar Association", "Housekeeping",
        "The Elevator (yes, the elevator)", "Inspector General's Office",
        "Your future self", "The basement (it learned to dial)",
        "A guest who checked out three years ago"
    };
    std::uniform_int_distribution<int> dist(0, names.size() - 1);
    return names[dist(rng)];
}

inline std::string getRandomCallMessage() {
    static std::mt19937 rng(std::random_device{}());
    static const std::vector<std::string> messages = {
        "The shower is singing again. It's in F minor and I want it to stop.",
        "Hello? I seem to have walked into my closet and now I'm... somewhere else?",
        "Could you send up more towels? Room 42 ate the last batch.",
        "There's a gentleman here insisting he's me from Thursday. He's not wrong, but I don't like his attitude.",
        "The room service menu just updated itself. Item #7 is now 'the concept of Thursday.' Should I order it?",
        "My key card opened someone else's room and now we're best friends. Is that normal?",
        "The television is showing tomorrow's news. The weather looks... alarming.",
        "I'd like to order room service, but the phone keeps answering ITSELF when I dial.",
        "There's a door in my bathroom that definitely wasn't there when I checked in. Should I... open it?",
        "Hello, this is Room 31 calling. I'm in Room 31 tomorrow and need to check in today. Can you arrange that?",
        "My luggage just apologized to me and left the room on its own. What is your luggage return policy?",
        "The ceiling has become the floor. Or possibly the floor has become the ceiling. Either way, I'm on the wrong one.",
        "I found a note under my pillow that says 'We've been trying to reach you about your room's extended warranty.' It's in my handwriting.",
        "Could you send maintenance? My room seems to be... molting.",
        "There's a second sun outside my window. Is that a hotel amenity?",
    };
    std::uniform_int_distribution<int> dist(0, messages.size() - 1);
    return messages[dist(rng)];
}

inline PhoneCall generatePhoneCall(GameState& state) {
    static std::mt19937 rng(std::random_device{}());
    PhoneCall call;
    call.callerName = getRandomCallerName();
    call.message = getRandomCallMessage();
    call.answered = false;
    
    std::uniform_int_distribution<int> roomDist(0, 1);
    if (roomDist(rng) && !state.checkedInGuests.empty()) {
        std::uniform_int_distribution<int> guestDist(0, state.checkedInGuests.size() - 1);
        call.fromRoom = state.checkedInGuests[guestDist(rng)].assignedRoom;
    }
    
    std::uniform_real_distribution<float> urgentDist(0.0f, 1.0f);
    call.urgent = urgentDist(rng) < 0.3f;
    
    return call;
}

inline std::string getPhoneAnsweredResponse(const PhoneCall& call) {
    if (call.urgent) {
        return "[URGENT] " + call.message;
    }
    return call.message;
}

} // namespace phone_system
