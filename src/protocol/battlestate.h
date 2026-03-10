#pragma once

#include <string>
#include <vector>

namespace pkm::protocol {

    struct Pokemon {
        std::string name;
        std::string ident;        // "p1: Donphan"
        int hp_current;
        int hp_max;
        bool active;
        bool fainted;
        std::vector<std::string> moves;
        std::string status;       // burned, paralyzed etc
    };

    
    struct BattleState {
        std::string room_id;
        int turn;
        
        // your side
        std::string your_name;
        std::vector<Pokemon> your_team;
        
        // opponent side  
        std::string opponent_name;
        std::vector<Pokemon> opponent_team;
        
        // current available moves from request
        struct MoveOption {
            std::string name;
            std::string id;
            int pp;
            int max_pp;
            bool disabled;
        };
        std::vector<MoveOption> available_moves;
        bool force_switch;
    };
    
}
