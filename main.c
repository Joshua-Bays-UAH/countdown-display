#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Used for getting the current time
#include <pthread.h>

#include "RS-232/rs232.h"

#include "defs.h"

int cport_nr = 16; // ttyUSB0
int bdrate = 9600;
unsigned bytesRecieved;
char rsmode[]={'8','N','1',0};
char cmdBuff[32];
long int offset = 0;
struct tm *tim;
_Bool timerMode = 0;
time_t initTime;
unsigned timerLen = 10;
_Bool stopwatchMode = 0;
long int remtime;

void *cmdInterpreter(void *vargp){
	while(1){
		start:
		if(RS232_OpenComport(cport_nr, bdrate, rsmode, 0)){
			//printf("Can not open comport\n");
			//return 1;
			//cport_nr++;
		//}else if(RS232_OpenComport(cport_nr, bdrate, rsmode, 0)){
			//cport_nr--;
			//printf("Can not open comport\n");
			//return 1;
			goto start;
		}
		SDL_Delay(500);
		bytesRecieved = RS232_PollComport(cport_nr, cmdBuff, 4095);
		RS232_CloseComport(cport_nr);
		if(bytesRecieved > 0){
			cmdBuff[bytesRecieved] = '\0';
			printf("C: %s\n", cmdBuff);
			if(strncmp(cmdBuff, "setc ", 5) == 0 && strlen(cmdBuff) >= 23){ // TODO: Add command prefix
				cmdBuff[19] = '\0';
				strptime(cmdBuff+5, "%m%d%Y%H%M%S", tim);
				printf("O: %lu\n", mktime(tim));
				offset = mktime(tim) - time(0);
			}else if(strncmp(cmdBuff, "sett ", 5) == 0 && strlen(cmdBuff) >= 13){
				timerLen = (cmdBuff[5] - '0') * 36000;
				timerLen += (cmdBuff[6] - '0') * 3600;
				timerLen += (cmdBuff[8] - '0') * 600;
				timerLen += (cmdBuff[9] - '0') * 60;
				timerLen += (cmdBuff[11] - '0') * 10;
				timerLen += (cmdBuff[12] - '0');
				initTime = time(0);
				remtime = timerLen - (time(0) - initTime);
				printf("T: %u\n", timerLen);
				stopwatchMode = 0;
				timerMode = 1;
			}else if(strncmp(cmdBuff, "sets", 4) == 0){
				timerMode = 1;
				stopwatchMode = 1;
				initTime = time(0);
				printf("Stopwatch\n");
			}
			for(unsigned i = 0; i < sizeof(cmdBuff); i++){ cmdBuff[i] = '\0'; }
		}
	}
}

int main(int argc, char *argv[]){
	unsigned st = time(0);
	initTime = time(0);
	
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
	
	SDL_Rect clockRect = {0, 0, 0, 0};
	SDL_GetWindowSize(window, &clockRect.w, &clockRect.h);
	clockRect.h /= 2;
	SDL_Rect timerRect = {0, 0, 0, 0};
	SDL_GetWindowSize(window, &timerRect.w, &timerRect.h);
	timerRect.h /= 2;
	timerRect.y = timerRect.h;
	
	_Bool hours[2];
	_Bool secs = 1;
	_Bool timerAlarm = 0;
	_Bool alarmClosed = 1;
	_Bool clockMode = 1;
	_Bool am;
	long int watchtime;
	unsigned hms[3];
	unsigned alarmLen = 5;
	unsigned timerChangeTime = 5;
	time_t now;
	time_t alarmTime;
	_Bool tPres = 0;
	_Bool cPres = 0;
	
	char clockStr[13];
	char timerStr[12];
	strcpy(timerStr, "!");
	
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
		am = 1;
		if(hms[0] == 0){ /*am = 1;*/ hms[0] = 12; }
		else if(hms[0] > 12){ am = 0; hms[0] -= 12; }
		
		sprintf(clockStr, " %u:", hms[0]);
		if(hms[1] < 10){ sprintf(clockStr + strlen(clockStr), "0%u", hms[1]); }
		else{ sprintf(clockStr + strlen(clockStr), "%u", hms[1]); }
		if(secs){
			if(hms[2] < 10){ sprintf(clockStr + strlen(clockStr), ":0%u", hms[2]); }
			else{ sprintf(clockStr + strlen(clockStr), ":%u", hms[2]); }
			//sprintf(clockStr+7+(hms[0]<10), " %s ", am ? "AM" : "PM");
		}
		sprintf(clockStr + strlen(clockStr), " %s ", am ? "AM" : "PM");
		
		if(timerMode){
			if(stopwatchMode){
				hours[1] = 1;
				watchtime = time(0) - initTime;
				hms[0] = floor((watchtime) / 3600);
				hms[1] = floor((watchtime - hms[0] * 3600) / 60);
				hms[2] = watchtime % 60;
				if(hms[0] == 0){ hours[1] = 0; }
				if(hours[1]){
					if(hms[0] < 10){ sprintf(timerStr, "0%u:", hms[0]); }
					else{ sprintf(timerStr, "%u:", hms[0]); }
				}else{ sprintf(timerStr, "   "); }
				if(hms[1] < 10){ sprintf(timerStr + strlen(timerStr), "0%u", hms[1]); }
				else{ sprintf(timerStr + strlen(timerStr), "%u", hms[1]); }
				if(hms[2] < 10){ sprintf(timerStr + strlen(timerStr), ":0%u", hms[2]); }
				else{ sprintf(timerStr + strlen(timerStr), ":%u", hms[2]); }
				if(!hours[1]){ sprintf(timerStr + strlen(timerStr), "   "); }
			}else{
				remtime = timerLen - (time(0) - initTime);
				if(remtime < 0){ remtime = 0; }
				hms[0] = floor((remtime) / 3600);
				hms[1] = floor((remtime - hms[0] * 3600) / 60);
				hms[2] = remtime % 60;
				hours[1] = hms[0] > 0;
				if(hours[1]){
					if(hms[0] < 10){ sprintf(timerStr, "0%u:", hms[0]); }
					else{ sprintf(timerStr, "%u:", hms[0]); }
				}else{ sprintf(timerStr, "   "); }
				if(hms[1] < 10){ sprintf(timerStr+3, "0%u", hms[1]); }
				else{ sprintf(timerStr+3, "%u", hms[1]); }
				if(hms[2] < 10){ sprintf(timerStr+5, ":0%u", hms[2]); }
				else{ sprintf(timerStr+5, ":%u", hms[2]); }
				if(!hours[1]){ sprintf(timerStr+8, "   "); }
			}
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
							timerMode = !timerMode;
							if(timerMode){
								initTime = time(0);
								timerLen = 12;
							}
							printf("Changed\n");
							/*
							if(mode == currenttime){
								timerLen = 5; initTime = time(0);
							}else{
								mode = currenttime;
								hours = 1;
							}
							*/
							/*
							if(timerMode){
								timerMode = 1;
								timerLen = 5;
								initTime = time(0);
								timerAlarm = 0;
								timerColor.r = DefaultTextR;
								timerColor.g = DefaultTextG;
								timerColor.b = DefaultTextB;
							}
							*/
							break;
						case SDL_SCANCODE_T:
							if(timerMode){
								stopwatchMode = !stopwatchMode;
							}
							if(!stopwatchMode){
								timerLen = 12;
							}
							if(stopwatchMode){
								initTime = time(0);
							}
							break;
						case SDL_SCANCODE_S:
							secs = !secs;
							break;
						case SDL_SCANCODE_H:
							break;
					}
					break;
			}
		}
		//if(timerAlarm){ SDL_SetRenderDrawColor(renderer, 0, 119, 200, 255); }
		//else{ SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); }
		//else{ SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); }
		
		
		SDL_RenderClear(renderer);

		if(timerMode){
			if(remtime <= timerChangeTime && !stopwatchMode){
				timerColor.r = 255;
				timerColor.g = 0;
				timerColor.b = 0;
			}else{
				timerColor.r = DefaultTextR;
				timerColor.g = DefaultTextG;
				timerColor.b = DefaultTextB;
			}
			//timerRect.w = (hours[1] ? SDL_GetWindowSurface(window)->w : .8 * SDL_GetWindowSurface(window)->w);
			//timerRect.y = (hours[1] ? timerRect.w : .125 * timerRect.w);
			timerSurface = TTF_RenderText_Solid(timerFont, timerStr, timerColor);
  			timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
			SDL_SetRenderTarget(renderer, timerTexture);
			SDL_RenderCopy(renderer, timerTexture, NULL, clockMode ? &timerRect : NULL);
			tPres = 1;
		}else{ tPres = 0; }
		
		if(clockMode){
			clockSurface = TTF_RenderText_Solid(clockFont, clockStr, clockColor);
			clockTexture = SDL_CreateTextureFromSurface(renderer, clockSurface);
			SDL_SetRenderTarget(renderer, clockTexture);
			SDL_RenderCopy(renderer, clockTexture, NULL, timerMode ? &clockRect : NULL);
			cPres = 1;
		}else{ cPres = 0; }
		
		SDL_RenderPresent(renderer);
		
		if(cPres){
			SDL_DestroyTexture(clockTexture);
			SDL_FreeSurface(clockSurface);
		}
		if(tPres){
			SDL_DestroyTexture(timerTexture);
			SDL_FreeSurface(timerSurface);
		}
	}
	return 0;
}
