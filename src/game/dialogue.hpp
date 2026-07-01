#pragma once

#include "game/types.hpp"
#include "hotel/rooms.hpp"
#include "hotel/guests.hpp"
#include <string>
#include <vector>
#include <random>

namespace dialogue_system {

inline std::string getGreeting(const Guest& guest) {
    switch (guest.type) {
        case GuestType::HUMAN:
            return "Hello! I have a reservation under " + guest.name + ".";
        case GuestType::VAMPIRE:
            return "Good evening. I trust my... accommodations are prepared?";
        case GuestType::GHOST:
            return "I'm terribly sorry, I seem to have... drifted in. Do you have any vacancies?";
        case GuestType::TIME_TRAVELER: {
            int years = guest.yearsFromFuture;
            if (guest.fromPast) {
                return "I say! Is this the Hotel " + std::to_string(abs(guest.yearsFromPast)) + " years in the future? Marvelous!";
            }
            return "Greetings from the year " + std::to_string(2024 + years) + "! I believe I've already checked in tomorrow?";
        }
        case GuestType::DOPPELGANGER:
            return "Excuse me... have you seen someone who looks exactly like me? I seem to have misplaced myself.";
        case GuestType::FAE:
            return "I have a reservation. Under my TRUE name, of course. You do have it, don't you?";
        case GuestType::SHADOW_PERSON:
            return "...*shifts ominously*... Room. Dark. Now.";
        case GuestType::BLOB:
            return "*gurgle* ...checking in... *slosh* ...need towels...";
        case GuestType::JUST_REALLY_WEIRD:
            return "Hi! Quick question: do your rooms come with extradimensional pockets, or do I need to bring my own?";
        case GuestType::INSPECTOR:
            return "Hotel Inspection Bureau. Don't mind me. Just... observing.";
    }
    return "Hello, I'd like a room.";
}

inline std::string getResponse(const Guest& guest, const std::string& action, const Room* room = nullptr) {
    static std::mt19937 rng(std::random_device{}());
    
    if (action == "ASSIGN_ROOM" && room) {
        if (room->rule == RoomRule::NORMAL) {
            if (guest.type == GuestType::TIME_TRAVELER) {
                return "Fascinating. In the future, this room is a broom closet. I approve of the upgrade.";
            }
            if (guest.type == GuestType::VAMPIRE) {
                return "Acceptable. Though the windows face east. I shall... manage.";
            }
            if (guest.type == GuestType::FAE) {
                return "A... normal room? How unexpectedly boring. I suppose it will do.";
            }
            return "Perfect, thank you! This looks lovely.";
        }
        
        if (room->rule == RoomRule::MIRRORLESS && guest.type == GuestType::VAMPIRE) {
            return "Ah! No mirrors! FINALLY! A hotelier who UNDERSTANDS! You shall receive a glowing review!";
        }
        if (room->rule == RoomRule::DOES_NOT_EXIST && guest.type == GuestType::GHOST) {
            return "A room that doesn't exist? For someone who technically doesn't exist either? It's PERFECT!";
        }
        if (room->rule == RoomRule::BIGGER_INSIDE && guest.type == GuestType::TIME_TRAVELER) {
            return "OH! A spatial anomaly! We have these in the future but they're much less charming. I love it!";
        }
        if (room->rule == RoomRule::DUPLICATES_OCCUPANT && guest.type == GuestType::DOPPELGANGER) {
            return "A duplication room?! This could... solve... or complicate... everything. I'll take it!";
        }
        
        std::vector<std::string> genericResponses = {
            "Oh, this room is... interesting. In a good way! Mostly.",
            "I think I saw the walls breathe. Is that normal? Never mind, don't answer that.",
            "The floor is where it should be, so we're off to a good start!",
            "Interesting decor choice. Very... surrealist.",
            "I'll take it! The strange humming is probably just the pipes. Right?",
            "This room has... character. I respect that.",
        };
        std::uniform_int_distribution<int> dist(0, genericResponses.size() - 1);
        return genericResponses[dist(rng)];
    }
    
    if (action == "GREET") {
        return getGreeting(guest);
    }
    
    if (action == "IMPATIENT") {
        std::vector<std::string> impatientLines = {
            "Is there a problem? I've been waiting for quite some time.",
            "How long does it take to find a room in this place?",
            "I do have other appointments, you know. Possibly. Time is a construct.",
            "*taps foot* The line is... not moving.",
            "I'm beginning to wonder if this hotel actually exists.",
        };
        std::uniform_int_distribution<int> dist(0, impatientLines.size() - 1);
        return impatientLines[dist(rng)];
    }
    
    if (action == "COMPLAINT") {
        std::vector<std::string> complaintLines = {
            "Excuse me! There's something in my room that shouldn't exist in this century!",
            "The wallpaper keeps changing patterns! I specifically requested the paisley!",
            "My room service order arrived before I ordered it. I find this deeply unsettling.",
            "There's a door in my closet that leads to... I don't know where. Should I be concerned?",
            "The minibar just asked me for career advice. I don't think appliances should do that.",
            "I came here to relax, not to question the fundamental nature of reality!",
        };
        std::uniform_int_distribution<int> dist(0, complaintLines.size() - 1);
        return complaintLines[dist(rng)];
    }
    
    if (action == "THANK") {
        std::vector<std::string> thanksLines = {
            "Thank you! You're far more competent than the last hotel I stayed at. The last one ate my cousin.",
            "Much appreciated! I'll be sure to mention you in my interdimensional travel blog.",
            "Excellent service! I'll recommend this place to everyone in my timeline.",
            "Thanks! This is exactly the kind of normal hotel experience I was hoping for.",
        };
        std::uniform_int_distribution<int> dist(0, thanksLines.size() - 1);
        return thanksLines[dist(rng)];
    }
    
    return "...";
}

inline std::string getRoomReaction(const Guest& guest, const Room& room) {
    std::string ruleName = room_system::getRuleName(room.rule);
    
    if (room.rule == RoomRule::DOES_NOT_EXIST) {
        if (guest.type == GuestType::GHOST) return guest.name + " seems delighted by Room 0.";
        return guest.name + " looks confused. \"But... where's the room?\"";
    }
    if (room.rule == RoomRule::DUPLICATES_OCCUPANT) {
        return guest.name + " glances at the mirror wall nervously. \"Why are there TWO of me in the reflection?\"";
    }
    if (room.rule == RoomRule::EATS_BAGGAGE) {
        return guest.name + " sets down their suitcase. It sinks slightly into the floor. \"Did the carpet just...?\"";
    }
    if (room.rule == RoomRule::BIGGER_INSIDE) {
        return guest.name + " opens the door and stares. \"...but the building isn't this big.\"";
    }
    if (room.rule == RoomRule::OPENS_YESTERDAY) {
        return "The door opens. It's still yesterday outside. " + guest.name + " waves at their past self.";
    }
    if (room.rule == RoomRule::TIMELOOP_HOUR) {
        return guest.name + " checks their watch. \"It was 3:14 AM when I came in. It's still 3:14 AM. I've been here for HOURS.\"";
    }
    if (room.rule == RoomRule::MULTIPLIES_BILL) {
        return guest.name + " reads the fine print. Their expression shifts from confused to horrified.";
    }
    
    return guest.name + " enters " + room.name + ". The room seems... " + ruleName + ".";
}

} // namespace dialogue_system
