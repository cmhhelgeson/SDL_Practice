#include "C:\SDL2-w64\include\SDL2\SDL.h"
#include "C:\SDL2-w64\include\SDL2\SDL_image.h"
#include "C:\SDL2-w64\include\SDL2\SDL_syswm.h"
#include "C:\SDL2-w64\include\SDL2\SDL_system.h"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <math.h>
#include "vec.h"

#if HANDMADE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define MAX_PLAYERS 2
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) *1024LL)
#define Gigabytes(Value) (Megabytes(Value) *1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

SDL_GameController *game_controllers[MAX_PLAYERS];
SDL_Haptic *rumblers[MAX_PLAYERS];

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const bool linux_flag = false;
const bool windows_flag = true;

static bool running = true;
static int num_players = 0;

class Triangle {
public: 
	Triangle() = default;
	Triangle(v2f _pos, v2f _v1, v2f _v2, v2f _v3) : 
		pos(_pos), v1(_v1), v2(_v2), v3(_v3) {}
public:
	v2f pos;
	v2f v1;
	v2f v2;
	v2f v3;
};

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



/*static void
SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer, sdl_graphics_buffer *Buffer)
{
	//Updates the texture of the screen with new data
    SDL_UpdateTexture(Buffer->texture,
                      0,
                      Buffer->mem,
                      Buffer->pitch);

	//copies texture to current render target
    SDL_RenderCopy(Renderer,
                   Buffer->texture,
                   0,
                   0);

	//Presents the graphics to the screen
    SDL_RenderPresent(Renderer);
} */

static void SDLProcessKeyPress(game_button_state *state, bool down) {
	Assert(state->ended_down != down);
	state->ended_down = down;
	++state->half_transition_count;
}


bool HandleEvent(SDL_Event *Event, game_controller_input *NewKeyboardController)
{
    bool ShouldQuit = false;
 
    switch(Event->type)
    {
        case SDL_QUIT:
        {
            printf("SDL_QUIT\n");
            ShouldQuit = true;
        } break;
        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode KeyCode = Event->key.keysym.sym;
            bool IsDown = (Event->key.state == SDL_PRESSED);
            bool WasDown = false;
            if (Event->key.state == SDL_RELEASED)
            {
                WasDown = true;
            }
            else if (Event->key.repeat != 0)
            {
                WasDown = true;
            }
            
            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (Event->key.repeat == 0)
            {
                if(KeyCode == SDLK_w)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveUp, IsDown);
                }
                else if(KeyCode == SDLK_a)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveLeft, IsDown);
                }
                else if(KeyCode == SDLK_s)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveDown, IsDown);
                }
                else if(KeyCode == SDLK_d)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveRight, IsDown);
                }
                else if(KeyCode == SDLK_q)
                {
                    SDLProcessKeyPress(&NewKeyboardController->LeftShoulder, IsDown);
                }
                else if(KeyCode == SDLK_e)
                {
                    SDLProcessKeyPress(&NewKeyboardController->RightShoulder, IsDown);
                }
                else if(KeyCode == SDLK_UP)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionUp, IsDown);
                }
                else if(KeyCode == SDLK_LEFT)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionLeft, IsDown);
                }
                else if(KeyCode == SDLK_DOWN)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionDown, IsDown);
                }
                else if(KeyCode == SDLK_RIGHT)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionRight, IsDown);
                }
                else if(KeyCode == SDLK_ESCAPE)
                {
                    printf("ESCAPE: ");
                    if(IsDown)
                    {
                        printf("IsDown ");
                    }
                    if(WasDown)
                    {
                        printf("WasDown");
                    }
                    printf("\n");
                }
                else if(KeyCode == SDLK_SPACE)
                {
                }
            }

            bool AltKeyWasDown = (Event->key.keysym.mod & KMOD_ALT);
            if (KeyCode == SDLK_F4 && AltKeyWasDown)
            {
                ShouldQuit = true;
            }

        } break;

        case SDL_WINDOWEVENT:
        {
            switch(Event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", Event->window.data1, Event->window.data2);
                } break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                } break;

                case SDL_WINDOWEVENT_EXPOSED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                } break;
            }
        } break;
    }
    
    return(ShouldQuit);
}

static float NormalizeInputToDeadzone(int16_t val, int16_t threshold) {
	float normalized = 0;

	if (val < threshold) {
		normalized = (float)(val + threshold) / (32768.0f - threshold); 
	} else if (val > threshold) {
		normalized = (float)(val - threshold) / (32768.0f - threshold); 
	}

	return normalized;
}

static void SDLOpenControllers() {
	//Game Controller API exists within the joystick API. As such we need to
	//loop through the joysticks and select only those that are controllers.
	int num_joysticks = SDL_NumJoysticks();
	int control_index = 0;
	for (int joystick_index = 0; joystick_index < num_joysticks; ++joystick_index) {
		if (!SDL_IsGameController(joystick_index)) {
			//not a controller, so return to loop beginning to find another.
			continue;
		}

		if (control_index >= MAX_PLAYERS) {
			//leave the loop as we have found enough controllers
			break;
		}

		//If there are yet more controllers, open joystick as controller.
		//Then increment controller index
		game_controllers[control_index] = 
			SDL_GameControllerOpen(joystick_index);
		//We need to get an sdl_joystick handle from the controller to
		//initialize rumble
		SDL_Joystick *joystick_handle = 
			SDL_GameControllerGetJoystick(game_controllers[control_index]);
		rumblers[control_index] = 
			SDL_HapticOpenFromJoystick(joystick_handle);
		num_players++;
		control_index++;
	}
 
}


static void SDLCloseControllers() {
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (game_controllers[i]) {
			if (rumblers[i]) {
				SDL_HapticClose(rumblers[i]);
			}
			SDL_GameControllerClose(game_controllers[i]);
		}
	}
}

static void SDLOpenController(int32_t joystick) {
	if (!SDL_IsGameController(joystick)) {
		return;
	}

	if (num_players >= MAX_PLAYERS) {
		return;
	}

	game_controllers[num_players] = 
		SDL_GameControllerOpen(joystick);
	SDL_Joystick *joystick_handle = 
		SDL_GameControllerGetJoystick(game_controllers[num_players]);
	rumblers[num_players] = 
		SDL_HapticOpenFromJoystick(joystick_handle);
	num_players++;
}

static void SDLCloseController(int32_t joystick) {
	//TODO: Decide if you would like to happen if player one leaves
	//Will player two slide into player one position? Define that behavior.
	if (joystick < 0 || joystick >= MAX_PLAYERS) {
		return;
	}
	if(game_controllers[joystick]) {
		if (rumblers[joystick]) {
			SDL_HapticClose(rumblers[joystick]);
		}
		SDL_GameControllerClose(game_controllers[joystick]);
	}
}

//This function will allows us to determine the ideal monitor
//display mode for our frame rate, as certain frame rates are only
//supported at certain resolutions
static int SDLGetWindowRefreshRate(SDL_Window *window) {
	SDL_DisplayMode display_mode;
	int display_index = SDL_GetWindowDisplayIndex(window);
	int refresh = 60;
	//Return default refresh rate if a monitor can't be found
	if (SDL_GetDesktopDisplayMode(display_index, &display_mode) != 0) {
		return refresh;
	}
	//return default refresh rate if the display has a refresh of 0
	if (display_mode.refresh_rate == 0) {
		return refresh;
	} 
	//else return the monitor's refresh rate
	return display_mode.refresh_rate;
}


sdl_window_dimension SDLGetWindowDimensions(SDL_Window *window) {
	sdl_window_dimension result;
	SDL_GetWindowSize(window, &result.w, &result.h);
	return result;
}


inline game_controller_input *GetController(game_input *input, int unsigned index) {
	Assert(index < ArrayCount(input->Controllers)); 

	game_controller_input *result = &input->Controllers[index];
	return result;
}

static void
SDLProcessGameControllerButton(game_button_state *OldState,
                               game_button_state *NewState,
                               bool Value)
{
    NewState->ended_down = Value;
    NewState->half_transition_count += ((NewState->ended_down == OldState->ended_down)?0:1);
}

static float SDLProcessGameControllerAxisValue(int16_t Value, int16_t DeadZoneThreshold) {
	float result = 0;
	if(Value < -DeadZoneThreshold) {
        result = (float)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    } else if(Value > DeadZoneThreshold) {
        result = (float)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(result);
}

void Draw_Sprite(SDL_Rect *source_rect, SDL_Rect *dest_rect, int scale, int end_frame, int ms_per_frame) {
    source_rect->x = source_rect->w * int(((SDL_GetTicks() / ms_per_frame) % end_frame));
    dest_rect->x = 0;
    dest_rect->y = source_rect->y;
	dest_rect->w = source_rect->w * scale;
	dest_rect->h = source_rect->h * scale;
}

/*void Draw_Triangle(SDL_Renderer *graphics, Triangle tri, float r = 0.0f) {
	tri.v1.x = tri.v1.x * cosf(r) - tri.v1.y * sinf(r);
	tri.v1. y= tri.v1.x * sinf(r) + tri.v1.y * cosf(r);

	tri.v2.x = tri.v2.x * cosf(r) - tri.v2.y * sinf(r);
	tri.v2.y= tri.v2.x * sinf(r) + tri.v2.y * cosf(r);

	tri.v3.x = tri.v3.x * cosf(r) - tri.v3.y * sinf(r);
	tri.v3.y= tri.v3.x * sinf(r) + tri.v3.y * cosf(r);
	
	v2i one = {tri.pos.x  + tri.v1.x, tri.pos.y + tri.v1.y};
	v2i two = {tri.pos.x  + tri.v2.x, tri.pos.y + tri.v2.y};
	v2i three = {tri.pos.x  + tri.v3.x, tri.pos.y + tri.v3.y};
	SDL_RenderDrawLine(graphics, one.x, one.y, two.x, two.y);
	SDL_RenderDrawLine(graphics, one.x, one.y, three.x, three.y);
	SDL_RenderDrawLine(graphics, two.x, two.y, three.x, three.y);
} */

int main(int argc, char ** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | 
		SDL_INIT_GAMECONTROLLER | 
		SDL_INIT_HAPTIC | 
		SDL_INIT_AUDIO ) < 0) {
		printf("SDL Initialization Failed: SDL_Error %s\n", SDL_GetError());
		return -1;
	}
	//this variable attempts to give an accurate impression of real world time
	uint64_t perf_frequency = 
		SDL_GetPerformanceFrequency();
	SDLOpenControllers();
    SDL_Window * window = SDL_CreateWindow("SDL2 line drawing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (window == NULL) {
		printf("Window Creation Failure: SDL_Error %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

    SDL_Renderer * graphics = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (graphics == NULL) {
		printf("Renderer Creation Failure: SDL_Erro %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}

	printf("The monitor's refresh rate is %d Hz \n", 
		SDLGetWindowRefreshRate(window));
	int target_fps = 60;
	/* if (SDLGetWindowRefreshRate(window) >= 120) {
		target_fps = 120;
	} */
	float frame_budget = 1.0f / 60.0f;

	//Get the dimensions of the window and resize the back buffer to 
	//correspond to the dimensions of the window, at least at the beginning
	//of the program
	sdl_window_dimension win_dimensions = SDLGetWindowDimensions(window);
	//SDLResizeTexture(&back_buffer, graphics, win_dimensions.w, win_dimensions.h);

	game_input input[2] = {};
	game_input *new_input = &input[0];
	game_input *old_input = &input[1];

	void *base_address = (void*)(0);

	/*game_memory gamemem;
	gamemem.permanent_storage_size = Megabytes(64);
	gamemem.transient_storage_size = Gigabytes(4);

	uint64_t total_storage = gamemem.permanent_storage_size +
		gamemem.transient_storage_size;

	gamemem.permanent_storage = VirtualAlloc(base_address, total_storage,
		MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE); 

	gamemem.transient_storage = (uint8*)(gamemem.permanent_storage) + gamemem.permanent_storage_size; */

	uint64_t last_counter = SDL_GetPerformanceCounter();
	uint64_t last_cycle_count = _rdtsc();

	//contains a collection of pixels
	SDL_Surface *temp_surface = IMG_Load("res/sPlayerRun_strip32.png");

	//Driver specific collection of pixel data?
	//TODO: Error checks for unsuccessful texture creation
	SDL_Texture *player_texture = SDL_CreateTextureFromSurface(graphics, temp_surface);
	SDL_Rect source_rect;
	SDL_Rect dest_rect;
    source_rect.w = 17;
    source_rect.h = 32;
    
	//SDL_QueryTexture(player_texture, NULL, NULL, &source_rect.w, &source_rect.h);
	//frees temp surface used to make texture
	SDL_FreeSurface(temp_surface);
	

    while (running)
    {
		//Set up Keyboards at index 0 of both new and old inputs
		game_controller_input *OldKeyboardController = GetController(old_input,0);
        game_controller_input *NewKeyboardController = GetController(new_input,0);
        *NewKeyboardController = {};

		for(int ButtonIndex = 0;
        	ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
            ++ButtonIndex) {
            	NewKeyboardController->Buttons[ButtonIndex].ended_down =
                OldKeyboardController->Buttons[ButtonIndex].ended_down;
        }
		SDL_Event event;
        while(SDL_PollEvent(&event)) {
			if (HandleEvent(&event, NewKeyboardController)) {
				running = false;
			}
		}
		//Handles input: think about putting in a different function
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (game_controllers[i] != 0 && 
				SDL_GameControllerGetAttached(game_controllers[i])) {
					game_controller_input *OldController = GetController(old_input,i+1);
                    game_controller_input *NewController = GetController(new_input,i+1);
					NewController->IsConnected = true;
                    
                //TODO: Do something with the DPad, Start and Selected?
                bool Up = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_UP);
                bool Down = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                bool Left = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                bool Right = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                bool Start = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_START);
                bool Back = SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_BACK);

				SDLProcessGameControllerButton(&(OldController->LeftShoulder),
                    	&(NewController->LeftShoulder), SDL_GameControllerGetButton(game_controllers[i], 
						SDL_CONTROLLER_BUTTON_LEFTSHOULDER));

                SDLProcessGameControllerButton(&(OldController->RightShoulder),
                               &(NewController->RightShoulder),
                               SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));

                SDLProcessGameControllerButton(&(OldController->ActionDown),
                               &(NewController->ActionDown),
                               SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_A));

                SDLProcessGameControllerButton(&(OldController->ActionRight),
                               &(NewController->ActionRight),
                               SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_B));

                SDLProcessGameControllerButton(&(OldController->ActionLeft),
                               &(NewController->ActionLeft),
                               SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_X));

                SDLProcessGameControllerButton(&(OldController->ActionUp),
                               &(NewController->ActionUp),
                               SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_Y));
							   
				NewController->StickAverageX = 
					SDLProcessGameControllerAxisValue(SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTX), 1);
                NewController->StickAverageY = 
					-SDLProcessGameControllerAxisValue(SDL_GameControllerGetAxis(game_controllers[i], SDL_CONTROLLER_AXIS_LEFTY), 1);
				if((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f)) {
                	 NewController->IsAnalog = true;
                }

                if(SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_UP))
                {
                	NewController->StickAverageY = 1.0f;
                    NewController->IsAnalog = false;
                }
                        
                if(SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                {
                    NewController->StickAverageY = -1.0f;
                    NewController->IsAnalog = false;
                }
                        
                if(SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT))
				{
                	NewController->StickAverageX = -1.0f;
                    NewController->IsAnalog = false;
                }
                        
                if(SDL_GameControllerGetButton(game_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                {
                	NewController->StickAverageX = 1.0f;
                    NewController->IsAnalog = false;
                }

                float Threshold = 0.5f;
                SDLProcessGameControllerButton(&(OldController->MoveLeft),
                                                       &(NewController->MoveLeft),
                                                       NewController->StickAverageX < -Threshold);
                SDLProcessGameControllerButton(&(OldController->MoveRight),
                                                       &(NewController->MoveRight),
                                                       NewController->StickAverageX > Threshold);
                SDLProcessGameControllerButton(&(OldController->MoveUp),
                                                       &(NewController->MoveUp),
                                                       NewController->StickAverageY < -Threshold);
                SDLProcessGameControllerButton(&(OldController->MoveDown),
                                                       &(NewController->MoveDown),
                                                       NewController->StickAverageY > Threshold);
			}
		}

		/*sdl_graphics_buffer current_buffer = {};
		current_buffer.mem = back_buffer.mem;
		current_buffer.width = back_buffer.width;
		current_buffer.height = back_buffer.height;
		current_buffer.pitch = back_buffer.pitch;
        */
		

		/*v2f vert_1_transform = 
			{vert_1.x * cosf(angle) - vert_1.y * sinf(angle),
		 	vert_1.x * sinf(angle) + vert_1.y * cosf(angle)};
		v2f vert_2_transform = 
			{vert_2.x * cosf(angle) - vert_2.y * sinf(angle),
		 	vert_2.x * sinf(angle) + vert_2.y * cosf(angle)};
		v2f vert_3_transform = 
			{vert_3.x * cosf(angle) - vert_3.y * sinf(angle),
		 	vert_3.x * sinf(angle) + vert_3.y * cosf(angle)};
		v2i v1 = {(int)vert_1_transform.x, (int) vert_2_transform.y};
		v2i v2 = {(int)vert_2_transform.x, (int) vert_2_transform.y};
		v2i v3 = {(int)vert_3_transform.x, (int) vert_3_transform.y}; */
 
        SDL_SetRenderDrawColor(graphics, 242, 242, 242, 255);
        SDL_RenderClear(graphics);


		SDL_SetRenderDrawColor(graphics, 0, 0, 0, 255);
        Draw_Sprite(&source_rect, &dest_rect, 4, 32, 80);

		SDL_RenderCopy(graphics, player_texture, &source_rect, &dest_rect);
 
        // render window
 
        SDL_RenderPresent(graphics);
    }
 
    // cleanup SDL

	SDLCloseControllers();

	SDL_DestroyTexture(player_texture);
 
    SDL_DestroyRenderer(graphics);
    SDL_DestroyWindow(window);
    SDL_Quit();
 
    return 0;
}


