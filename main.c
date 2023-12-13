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
#define WindowFlags SDL_WINDOW_BORDERLESS // | SDL_WINDOW_RESIZABLE

int main(int argc, char *argv[]){
	SDL_Renderer *renderer;
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){ printf("Couldn't initialize SDL: %s\n", SDL_GetError()); return 1; }
	if(TTF_Init() < 0){ printf("Couldn't initialize TTF: %s\n", SDL_GetError()); return 1; }
	window = SDL_CreateWindow(WinName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, WindowFlags);
	if(!window){ printf("Failed to open %d x %d window: %s\n", WinWidth, WinHeight, SDL_GetError()); return 1; }
	renderer = SDL_CreateRenderer(window, -1, RendererFlags);
	if(!renderer){ printf("Failed to create renderer: %s\n", SDL_GetError()); return 1; }
	
	TTF_Font *countdownFont = TTF_OpenFont(FontFileName, 120);
	SDL_Color countdownColor = {255, 255, 255, 255};
	SDL_Surface *countdownText = TTF_RenderText_Solid(countdownFont, "TeSt", countdownColor);
	SDL_Texture *countdownTexture = SDL_CreateTextureFromSurface(renderer, countdownText);
	
	SDL_Rect countdownRect;
	countdownRect.x = 0; countdownRect.y = 0;
	countdownRect.w = WinWidth; countdownRect.h = WinHeight;
	
	long int kTimer = 0;
	char ctStr[16];
	SDL_Event event;
	
	while(1){
		for(unsigned i = 0; i < sizeof(ctStr); i++){ ctStr[i] = '\0'; }
		//strcpy(ctStr, "Josh");
		sprintf(ctStr, "%lu", time(0));
		
		countdownText = TTF_RenderText_Solid(countdownFont, ctStr, countdownColor);
		countdownTexture = SDL_CreateTextureFromSurface(renderer, countdownText);
		SDL_SetRenderTarget(renderer, countdownTexture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, countdownTexture, NULL, NULL);
		
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					exit(0);
					break;
			}
		}
	
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(countdownTexture);
		SDL_FreeSurface(countdownText);
		SDL_Delay(16);
	}
	
	return 0;
}

