#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>

#define WinWidth 960
#define WinHeight 720
#define WinName "CountDown"

//#define FontFileName "fonts/Jost/static/Jost-Medium.ttf"
//#define FontFileName "fonts/Jost/static/Jost-Black.ttf"
#define FontFileName "fonts/Silkscreen/Silkscreen-Regular.ttf"

#define RendererFlags SDL_RENDERER_ACCELERATED
//#define WindowFlags SDL_WINDOW_BORDERLESS
#define WindowFlags SDL_WINDOW_FULLSCREEN_DESKTOP

enum Modes{ currenttime, timer };

int main(int argc, char *argv[]){
	SDL_Renderer *renderer;
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){ printf("Couldn't initialize SDL: %s\n", SDL_GetError()); return 1; }
	if(TTF_Init() < 0){ printf("Couldn't initialize TTF: %s\n", SDL_GetError()); return 1; }
	window = SDL_CreateWindow(WinName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, WindowFlags);
	if(!window){ printf("Failed to open %d x %d window: %s\n", WinWidth, WinHeight, SDL_GetError()); return 1; }
	renderer = SDL_CreateRenderer(window, -1, RendererFlags);
	if(!renderer){ printf("Failed to create renderer: %s\n", SDL_GetError()); return 1; }
	
	SDL_ShowCursor(0);
	SDL_Event event;
	
	TTF_Font *countdownFont = TTF_OpenFont(FontFileName, 120);
	SDL_Color countdownColor = {255, 255, 255, 255};
	SDL_Surface *countdownSurface = TTF_RenderText_Solid(countdownFont, "TeSt", countdownColor);
	SDL_Texture *countdownTexture = SDL_CreateTextureFromSurface(renderer, countdownSurface);
	
	SDL_Rect countdownRect;
	countdownRect.x = 0; countdownRect.y = 0;
	countdownRect.w = WinWidth; countdownRect.h = WinHeight;
	
	char mode = currenttime;
	
	time_t now;
	struct tm *tm_struct;
	char ctStr[16];
	
	while(1){
		if(mode == currenttime){
			now = time(NULL); tm_struct = localtime(&now);
			sprintf(ctStr, "%u:%u:%u", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
		}
		
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					exit(0);
					break;
			}
		}
	
		countdownSurface = TTF_RenderText_Solid(countdownFont, ctStr, countdownColor);
		countdownTexture = SDL_CreateTextureFromSurface(renderer, countdownSurface);
		SDL_SetRenderTarget(renderer, countdownTexture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, countdownTexture, NULL, NULL);
		
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(countdownTexture);
		SDL_FreeSurface(countdownSurface);
		SDL_Delay(16);
	}
	
	return 0;
}

