#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define _XOPEN_SOURCE
//#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Used for getting the current time
#include <pthread.h>

#include "RS-232/rs232.h"

#define WinWidth 1
#define WinHeight 1
#define WinName "Charger Time"

//#define FontFileName "fonts/Jost/static/Jost-Medium.ttf"
//#define FontFileName "fonts/Jost/static/Jost-Black.ttf"
//#define FontFileName "fonts/Silkscreen/Silkscreen-Regular.ttf"
//#define FontFileName "fonts/Electrolize/Electrolize-Regular.ttf"
#define FontFileName "/home/josh/School/ETLC/countdown-display/fonts/Nunito/NunitoSans-VariableFont_YTLC,opsz,wdth,wght.ttf"

//#define AlarmFilename "sounds/RR.wav"
#define AlarmFilename "/home/josh/School/ETLC/countdown-display/sounds/chime.wav"

//#define RendererFlags SDL_RENDERER_ACCELERATED
#define RendererFlags SDL_RENDERER_PRESENTVSYNC
//#define WindowFlags SDL_WINDOW_BORDERLESS
#define WindowFlags SDL_WINDOW_FULLSCREEN_DESKTOP

#define ClockSize 900 // Font size of display
#define TimerSize 750 // Font size of display

#define DefaultTextR 0
#define DefaultTextG 119
#define DefaultTextB 200
#define AlertTextR 255
#define AlertTextG 0
#define AlertTextB 0

int cport_nr = 16; // ttyUSB0
int bdrate = 9600;
unsigned bytesRecieved;
char rsmode[]={'8','N','1',0};
char cmdBuff[32];
long int offset = 0;
struct tm *tim;

void *cmdInterpreter(void *vargp){
	while(1){
		start:
		if(RS232_OpenComport(cport_nr, bdrate, rsmode, 0)){
			printf("Can not open comport\n");
			//return 1;
			goto start;
		}
		SDL_Delay(60);
		bytesRecieved = RS232_PollComport(cport_nr, cmdBuff, 4095);
		RS232_CloseComport(cport_nr);
		if(bytesRecieved > 0){
			cmdBuff[bytesRecieved] = '\0';
			printf("C: %s\n", cmdBuff);
		}
		if(bytesRecieved == 18 && strcmp(cmdBuff+14, "0000") == 0){ // TODO: Add command prefix
			/*
			if(strncmp(cmdBuff, "sett ", 5) == 0){
				timer = 1;
				timerLen = (cmdBuff[5] - '0') * 36000;
				timerLen += (cmdBuff[6] - '0') * 3600;
				timerLen += (cmdBuff[8] - '0') * 600;
				timerLen += (cmdBuff[9] - '0') * 60;
				timerLen += (cmdBuff[11] - '0') * 10;
				timerLen += (cmdBuff[12] - '0');
			}else if(strncmp(cmdBuff, "setc ", 5) == 0){
				*/
				//sscanf(cmdBuff + 5, "%li", &offset);
				cmdBuff[15] = '\0';
				strptime(cmdBuff, "%m%d%Y%H%M%S", tim);
				printf("O: %lu\n", mktime(tim));
				offset = mktime(tim) - time(0);
			//}
			for(unsigned i = 0; i < sizeof(cmdBuff); i++){ cmdBuff[i] = '\0'; }
		}
	}
}

int main(int argc, char *argv[]){
	unsigned st = time(0);
	
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
	
	SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;
	SDL_AudioDeviceID deviceId;

	TTF_Font *clockFont = TTF_OpenFont(FontFileName, ClockSize);
	TTF_Font *timerFont = TTF_OpenFont(FontFileName, TimerSize);
	SDL_Color timerColor = {DefaultTextR, DefaultTextG, DefaultTextB, 255};
	SDL_Color clockColor = {DefaultTextR, DefaultTextG, DefaultTextB, 255};
	SDL_Surface *clockSurface, *timerSurface;
	SDL_Texture *clockTexture, *timerTexture;
	
	_Bool hours = 1;
	_Bool secs = 1;
	_Bool timerAlarm = 0;
	_Bool alarmClosed = 1;
	_Bool timer = 0;
	long int remtime;
	unsigned hms[3];
	unsigned alarmLen = 5;
	unsigned timerLen = 5;
	unsigned timerChangeTime = 60;
	time_t now;
	time_t initTime = time(0);
	time_t alarmTime;
	
	char clockStr[9];
	char timerStr[9];
	
	pthread_t cmdThread;
	pthread_create(&cmdThread, NULL, cmdInterpreter, NULL);
	
	// Main loop
	while(1){

		// Create the clock string
		now = time(0) + offset;
		tim = localtime(&now);
		hms[0] = tim->tm_hour;
		hms[1] = tim->tm_min;
		hms[2] = tim->tm_sec;
		if(hms[0] == 0){ hms[0] = 12; }
		else if(hms[0] > 12){ hms[0] -= 12; }
		if(hours){
			if(hms[0] < 10){ sprintf(clockStr, "0%u:", hms[0]); }
			else{ sprintf(clockStr, "%u:", hms[0]); }
		}
		if(hms[1] < 10){ sprintf(clockStr+3*hours, "0%u", hms[1]); }
		else{ sprintf(clockStr+3*hours, "%u", hms[1]); }
		if(secs){
			if(hms[2] < 10){ sprintf(clockStr+3*hours+2, ":0%u", hms[2]); }
			else{ sprintf(clockStr+3*hours+2, ":%u", hms[2]); }
		}
		
		// Window Events
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					exit(0);
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.scancode){
						case SDL_SCANCODE_M:
							/*
							if(mode == currenttime){
								mode = timer;
								timerLen = 5; initTime = time(0);
							}else{
								mode = currenttime;
								hours = 1;
							}
							*/
							if(timer){
								timer = 1;
								timerLen = 5;
								initTime = time(0);
								timerAlarm = 0;
								timerColor.r = DefaultTextR;
								timerColor.g = DefaultTextG;
								timerColor.b = DefaultTextB;
							}
							break;
						case SDL_SCANCODE_S:
							secs = !secs;
							break;
						case SDL_SCANCODE_H:
							/*
							if(mode == timer){
								hours = !hours;
							}
							*/
							break;
					}
					break;
			}
		}

		clockSurface = TTF_RenderText_Solid(clockFont, clockStr, clockColor);
		clockTexture = SDL_CreateTextureFromSurface(renderer, clockSurface);
		SDL_SetRenderTarget(renderer, clockTexture);
		if(timerAlarm){ SDL_SetRenderDrawColor(renderer, 0, 119, 200, 255); }
		else{ SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); }
		//else{ SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); }
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, clockTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(clockTexture);
		SDL_FreeSurface(clockSurface);
	}
	return 0;
}
