#pragma once

struct game_button_state {
	int half_transition_count;
	bool ended_down;
};

struct game_controller_input
{
    bool IsConnected;
    bool IsAnalog;    
    float StickAverageX;
    float StickAverageY;
    
    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;
            
            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;

            // NOTE(casey): All buttons must be added above this line
            
            game_button_state Terminator;
        };
    };
};

struct game_input
{
    // TODO(casey): Insert clock values here.    
    game_controller_input Controllers[3];
};


struct sdl_window_dimension {
	int w;
	int h;
};