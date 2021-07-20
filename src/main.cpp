#include "C:\SDL2-w64\include\SDL2\SDL.h"
#include "C:\SDL2-w64\include\SDL2\SDL_image.h"
#include "C:\SDL2-w64\include\SDL2\SDL_syswm.h"
#include "C:\SDL2-w64\include\SDL2\SDL_system.h"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\Vec2.h"
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\types.h"
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\control.h"


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

static Vec2 mouse_pos;



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
        case SDL_MOUSEMOTION: {
            mouse_pos.x = Event->motion.x;
            mouse_pos.y = Event->motion.y;
        }
    }
    
    return(ShouldQuit);
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



void Draw_Sprite(SDL_Rect *source_rect, SDL_Rect *dest_rect, int scale, int end_frame, int ms_per_frame) {
    source_rect->x = source_rect->w * int(((SDL_GetTicks() / ms_per_frame) % end_frame));
    dest_rect->x = 0;
    dest_rect->y = source_rect->y;
	dest_rect->w = source_rect->w * scale;
	dest_rect->h = source_rect->h * scale;
}


void Draw_Line(SDL_Renderer *graphics, Vec2 start, Vec2 end) {
    SDL_RenderDrawLine(graphics, start.x, start.y, end.x, end.y);
}




int main(int argc, char ** argv)
{
    //BEGIN SETUP
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
	float frame_budget = 1.0f / 60.0f;

	sdl_window_dimension win_dimensions = SDLGetWindowDimensions(window);

	game_input input[2] = {};
	game_input *new_input = &input[0];
	game_input *old_input = &input[1];

	void *base_address = (void*)(0);

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
    
	SDL_FreeSurface(temp_surface);
	

    while (running)
    {
		SDL_Event event;
        while(SDL_PollEvent(&event)) {
			if (HandleEvent(&event, NewKeyboardController)) {
				running = false;
			}
		}
		
        SDL_SetRenderDrawColor(graphics, 0, 0, 0, 255);
        SDL_RenderClear(graphics);


        SDL_SetRenderDrawColor(graphics, 255, 255, 255, 255);
        Vec2 v;
        v.x = 100; 
        v.y = 100;
        Draw_Line(graphics, v, mouse_pos);


		//SDL_SetRenderDrawColor(graphics, 0, 255, 255, 255);
        Draw_Sprite(&source_rect, &dest_rect, 4, 32, 80);

        SDL_RenderCopyEx(graphics, player_texture, &source_rect, &dest_rect, 30, 0, SDL_FLIP_VERTICAL);

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


