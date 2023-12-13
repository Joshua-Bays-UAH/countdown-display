#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <time.h> // Used for getting the current time

#define WinWidth 960
#define WinHeight 720
#define WinName "Charger Time"

//#define FontFileName "fonts/Jost/static/Jost-Medium.ttf"
//#define FontFileName "fonts/Jost/static/Jost-Black.ttf"
//#define FontFileName "fonts/Silkscreen/Silkscreen-Regular.ttf"
#define FontFileName "fonts/Electrolize/Electrolize-Regular.ttf"

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
	
	TTF_Font *dispFont = TTF_OpenFont(FontFileName, 140);
	SDL_Color dispColor = {255, 255, 255, 255};
	SDL_Surface *dispSurface;
	SDL_Texture *dispTexture;
	
	/*
	SDL_Rect dispRect;
	dispRect.x = 0; dispRect.y = 0;
	dispRect.w = WinWidth; dispRect.h = WinHeight;
	*/
	
	_Bool secs = 0;
	char mode = currenttime;
	
	time_t now;
	struct tm *tm_struct;
	char ctStr[16];
	unsigned hms[3];
	
	unsigned dispRect;
	
	while(1){
		if(mode == currenttime){
			now = time(NULL); tm_struct = localtime(&now);
			hms[0] = tm_struct->tm_hour; hms[1] = tm_struct->tm_min; hms[2] = tm_struct->tm_sec;
			
			if(hms[0] < 10){ sprintf(ctStr, "0%u", hms[0]); }
			else{ sprintf(ctStr, "%u", hms[0]); }
			
			if(hms[1] < 10){ sprintf(ctStr+2, ":0%u", hms[1]); }
			else{ sprintf(ctStr+2, ":%u", hms[1]); }
			
			if(secs){
				if(hms[2] < 10){ sprintf(ctStr+5, ":0%u", hms[2]); }
				else{ sprintf(ctStr+5, ":%u", hms[2]); }
			}
			
		}
		
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					exit(0);
					break;
			}
		}
	
		dispSurface = TTF_RenderText_Solid(dispFont, ctStr, dispColor);
		dispTexture = SDL_CreateTextureFromSurface(renderer, dispSurface);
		SDL_SetRenderTarget(renderer, dispTexture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, dispTexture, NULL, NULL);
		
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(dispTexture);
		SDL_FreeSurface(dispSurface);
	}
	
	return 0;
}

