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

//#define AlarmFilename "sounds/RR.wav"
#define AlarmFilename "sounds/chime.wav"

#define RendererFlags SDL_RENDERER_ACCELERATED
//#define WindowFlags SDL_WINDOW_BORDERLESS
#define WindowFlags SDL_WINDOW_FULLSCREEN_DESKTOP

#define DispSize 140 // Font size of display

#define TimerChangeTime 300 // 5 Minutes
#define DefaultTextR 0
#define DefaultTextG 199
#define DefaultTextB 200
#define AlertR 255
#define AlertG 0
#define AlertB 0

enum Modes{ currenttime, timer };

int main(int argc, char *argv[]){
	SDL_Renderer *renderer;
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){ printf("Couldn't initialize SDL: %s\n", SDL_GetError()); return 1; }
	if(SDL_Init(SDL_INIT_AUDIO) < 0){ printf("Couldn't initialize Audio: %s\n", SDL_GetError()); return 1; }
	if(TTF_Init() < 0){ printf("Couldn't initialize TTF: %s\n", SDL_GetError()); return 1; }
	window = SDL_CreateWindow(WinName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, WindowFlags);
	if(!window){ printf("Failed to open %d x %d window: %s\n", WinWidth, WinHeight, SDL_GetError()); return 1; }
	renderer = SDL_CreateRenderer(window, -1, RendererFlags);
	if(!renderer){ printf("Failed to create renderer: %s\n", SDL_GetError()); return 1; }
	
	SDL_ShowCursor(0);
	SDL_Event event;
	
	TTF_Font *dispFont = TTF_OpenFont(FontFileName, DispSize);
	SDL_Color dispColor = {DefaultTextR, DefaultTextG, DefaultTextB, 255};
	SDL_Surface *dispSurface;
	SDL_Texture *dispTexture;
	
	/*
	SDL_Rect dispRect;
	dispRect.x = 0; dispRect.y = 0;
	dispRect.w = WinWidth; dispRect.h = WinHeight;
	*/
	
	SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;
	
	_Bool secs = 1;
	//char mode = currenttime;
	char mode = timer;
	
	time_t now;
	struct tm *tm_struct;
	char dispStr[16];
	unsigned hms[3];
	
	unsigned dispRect;
	
	time_t initTime = time(0);
	//unsigned timerLen = 120 * 3600 + 61;
	unsigned timerLen = 5;
	long int remtime;
	_Bool timerAlarm = 0;
	
	while(1){
		if(mode == currenttime){
			now = time(NULL); tm_struct = localtime(&now);
			hms[0] = tm_struct->tm_hour; hms[1] = tm_struct->tm_min; hms[2] = tm_struct->tm_sec;
			
			if(hms[0] < 10){ sprintf(dispStr, "0%u", hms[0]); }
			else{ sprintf(dispStr, "%u", hms[0]); }
			
			if(hms[1] < 10){ sprintf(dispStr+2, ":0%u", hms[1]); }
			else{ sprintf(dispStr+2, ":%u", hms[1]); }
			
			if(secs){
				if(hms[2] < 10){ sprintf(dispStr+5, ":0%u", hms[2]); }
				else{ sprintf(dispStr+5, ":%u", hms[2]); }
			}
		}else if(mode == timer){
			remtime = timerLen - (time(0) - initTime);
			if(remtime > 0){
				hms[0] = floor((remtime) / 3600);
				hms[1] = floor((remtime - hms[0] * 3600) / 60);
				hms[2] = remtime % 60;
				if(remtime <= TimerChangeTime){
					dispColor.r = 255;
					dispColor.g = 0;
					dispColor.b = 0;
				}
			}else{
				remtime = 0;
				hms[0] = 0; hms[1] = 0; hms[2] = 0;
				if(!timerAlarm){
					timerAlarm = 1;
					SDL_LoadWAV(AlarmFilename, &wavSpec, &wavBuffer, &wavLength);
					SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
					SDL_QueueAudio(deviceId, wavBuffer, wavLength);
					SDL_PauseAudioDevice(deviceId, 0);
					dispColor.r = 255; dispColor.g = 255; dispColor.b = 255;
					//dispColor.r = 0; dispColor.g = 0; dispColor.b = 0;
				}
	
			}
			if(hms[0] < 10){ sprintf(dispStr, "0%u", hms[0]); }
			else{ sprintf(dispStr, "%u", hms[0]); }
			
			if(hms[1] < 10){ sprintf(dispStr+2, ":0%u", hms[1]); }
			else{ sprintf(dispStr+2, ":%u", hms[1]); }
			
			if(secs){
				if(hms[2] < 10){ sprintf(dispStr+5, ":0%u", hms[2]); }
				else{ sprintf(dispStr+5, ":%u", hms[2]); }
			}
		}
		
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					exit(0);
					break;
			}
		}
	
		dispSurface = TTF_RenderText_Solid(dispFont, dispStr, dispColor);
		dispTexture = SDL_CreateTextureFromSurface(renderer, dispSurface);
		SDL_SetRenderTarget(renderer, dispTexture);
		if(timerAlarm){ SDL_SetRenderDrawColor(renderer, 0, 119, 200, 255); }
		else{ SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); }
		//else{ SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); }
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, dispTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(dispTexture);
		SDL_FreeSurface(dispSurface);
	}
	
	return 0;
}

