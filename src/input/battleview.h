#pragma once 

// input/IBattleView.hpp
class IBattleView {
public:
    virtual ~IBattleView() = default;
    
    // when state updates and view should refresh
    virtual void onStateUpdate(const BattleState& state) = 0;
    
    // NOTE: when it's the player's turn to pick
    // implementation decides how to get the choice
    // headless picks automatically, CLI prompts user
    virtual void onMoveRequest(const BattleState& state,
                               std::function<void(std::string)> respond) = 0;
    
    virtual void onSwitchRequest(const BattleState& state,
                                 std::function<void(std::string)> respond) = 0;
    
    virtual void onBattleEnd(const std::string& winner) = 0;
};

