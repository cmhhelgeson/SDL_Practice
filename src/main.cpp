#include "C:\SDL2-w64\include\SDL2\SDL.h"
#include "C:\SDL2-w64\include\SDL2\SDL_image.h"
#include "C:\SDL2-w64\include\SDL2\SDL_syswm.h"
#include "C:\SDL2-w64\include\SDL2\SDL_system.h"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <map>
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\Vec2.h"
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\types.h"
#include "C:\Users\Christian Helgeson\Desktop\Development\Game Development\ChiliTomatoNoodle Projects\DirectX 3D 11 Practice\DirectX with SDL\include\control.h"

const int SCREEN_WIDTH = 1281;
const int SCREEN_HEIGHT = 640;

const bool linux_flag = false;
const bool windows_flag = true;

static bool running = true;
static int num_players = 0;

static Vec2 mouse_pos; 

const static uint8_t *keystate;

//Args



/*static void SDLProcessKeyPress(game_button_state *state, bool down) {
	Assert(state->ended_down != down);
	state->ended_down = down;
	++state->half_transition_count;
} */


bool isKeyDown(SDL_Scancode pScan) {
    if (keystate != 0) {
        if (keystate[pScan] == 1) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool HandleEvent(SDL_Event *Event)// game_controller_input *NewKeyboardController)
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
            keystate = SDL_GetKeyboardState(0);
            if (isKeyDown(SDL_SCANCODE_ESCAPE)) {
                ShouldQuit = true;
            }
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


/*!
 * Returns the window dimensions of an SDL_Window
 * @param       *window     An SDL_Window struct
 * @result      A struct holding the window dimensions
 * 
 */
sdl_window_dimension SDLGetWindowDimensions(SDL_Window *window) {
	sdl_window_dimension result;
	SDL_GetWindowSize(window, &result.w, &result.h);
	return result;
}

bool Load_Texture(std::map<std::string, SDL_Texture*> *map, 
    std::string file, SDL_Renderer *graphics, std::string id) {

    SDL_Surface *temp_surface = IMG_Load(file.c_str());
    if (temp_surface == 0) {
        return false;
    } 
    SDL_Texture *texture;
    SDL_CreateTextureFromSurface(graphics, temp_surface);
    SDL_FreeSurface(temp_surface);
    if (texture != 0) {
        map->emplace(id, texture);
        return true;
    } else {
        return false;
    }
} 



/*!
 * @function
*/
void Draw_Sprite(SDL_Rect *source_rect, SDL_Rect *dest_rect, int scale, int end_frame, int ms_per_frame) {
    source_rect->x = source_rect->w * int(((SDL_GetTicks() / ms_per_frame) % end_frame));
    dest_rect->x = 0;
    dest_rect->y = source_rect->y;
	dest_rect->w = source_rect->w * scale;
	dest_rect->h = source_rect->h * scale;
}



/*!
 * Draws a Line to the screen
 * @param   graphics    A pointer to the SDL Renderer
 * @param   start       A 2D Vector that begins the line
 * @param   end         A 2D Vector that ends the line
 *     
*/
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
    SDL_Window * window = SDL_CreateWindow("SDL2 line drawing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (window == NULL) {
		printf("Window Creation Failure: SDL_Error %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

    //this variable attempts to give an accurate impression of real world time
	uint64_t perf_frequency = 
		SDL_GetPerformanceFrequency();

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

	uint64_t last_counter = SDL_GetPerformanceCounter();
	uint64_t last_cycle_count = _rdtsc();
	//contains a collection of pixels
    std::map<std::string, SDL_Texture*> texture_map;
    bool success = Load_Texture(&texture_map, "res/sPlayerRun_strip32.png", graphics, "run");
	/*SDL_Surface *temp_surface = IMG_Load("res/sPlayerRun_strip32.png");

	//Driver specific collection of pixel data?
	//TODO: Error checks for unsuccessful texture creation
	SDL_Texture *player_texture = SDL_CreateTextureFromSurface(graphics, temp_surface); */
	SDL_Rect source_rect;
	SDL_Rect dest_rect;
    source_rect.w = 17;
    source_rect.h = 32;

    int velocity = 2;
    int position = 0;


    
	//SDL_FreeSurface(temp_surface);
	
    while (running)
    {
		SDL_Event event;
        while(SDL_PollEvent(&event)) {
			if (HandleEvent(&event)) {  
				running = false;
			}
		}
		
        SDL_SetRenderDrawColor(graphics, 0, 0, 0, 255);
        SDL_RenderClear(graphics);

        SDL_SetRenderDrawColor(graphics, 255, 255, 255, 255);
        Vec2 v;
        Vec2 zeta;
        v.x = 100;
        v.y = 100;
        if (isKeyDown(SDL_SCANCODE_RIGHT)) {
            velocity = 2;
        } else {
            velocity = 0;
        }
        Draw_Line(graphics, v, mouse_pos);


		//SDL_SetRenderDrawColor(graphics, 0, 255, 255, 255);
        Draw_Sprite(&source_rect, &dest_rect, 4, 32, 80);

        SDL_RenderCopyEx(graphics, map["run"], &source_rect, &dest_rect, 0, 0, SDL_FLIP_VERTICAL);

        // render window
 
        SDL_RenderPresent(graphics);
    }
 
    // cleanup SDL
    std::map<std::string, int>::iterator it = texture_map.begin();
    while (it != texture_map.end()) {
        SDL_DestroyTexture(it->second);
    }
    while (!texture_map.empty()) {
        texture_map.clear();
    }
 
    SDL_DestroyRenderer(graphics);
    SDL_DestroyWindow(window);
    SDL_Quit();
	
	//Fun in the sun
 
    return 0;
}




