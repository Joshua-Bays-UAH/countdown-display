#include <SDL2/SDL.h> /* Graphics rendering */
#include <SDL2/SDL_ttf.h> /* Font rendering */

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "RS-232/rs232.h" /* Serial reading */

#include "defs.h"

/* Setup initial modes */
_Bool clockMode = 1; /* Clock displayed */
_Bool timerMode = 0; /* Timer/stopwatch displayed/running */
_Bool stopwatchMode = 0; /* timer or stopwatch enabled (both cannot be simeltaneously run) */

/* Global clock variables */
long int offset = 0; /* Difference between computer time and DMPS time */
struct tm *tim; /* Structure to convert string better time */
_Bool secs = 1; /* Whether or not the clock will display seconds */

/* Global timer/stopwatch variables */
time_t initTime; /* Inital time for timer/stopwatch start */
unsigned timerLen = 7; /* Length of timer (seconds) */
long int timerRem = 0; /* Time remaining on timer */
long int watchtime = 0; /* Stopwatch runtime */
_Bool timerPause = 0; /* If timer is paused */
_Bool stopwatchPause = 0; /* If stopwatch is paused */
_Bool timerAlarm = 0; /* If the timer alarm has been played */
_Bool alarmPlay = 1; /* Toggle if alert is played */

/* Process that reads/interprets serial commands from DMPS */
void *cmdInterpreter(void *vargp);

int main(int argc, char *argv[]){
	initTime = time(0); /* set initTime to prevent errors */
	
	/* SDL window setup */
	SDL_Renderer *renderer;
	SDL_Window *window;
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0){ printf("Couldn't initialize SDL: %s\n", SDL_GetError()); return 1; }
	if(SDL_Init(SDL_INIT_AUDIO) < 0){ printf("Couldn't initialize Audio: %s\n", SDL_GetError()); return 1; }
	if(TTF_Init() < 0){ printf("Couldn't initialize TTF: %s\n", SDL_GetError()); return 1; }
	window = SDL_CreateWindow(WinName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, WindowFlags);
	if(!window){ printf("Failed to open %d x %d window: %s\n", WinWidth, WinHeight, SDL_GetError()); return 1; }
	renderer = SDL_CreateRenderer(window, -1, RendererFlags);
	if(!renderer){ printf("Failed to create renderer: %s\n", SDL_GetError()); return 1; }
	
	SDL_ShowCursor(0); /* Do not display the mouse */
	SDL_Event event; /* Event handler */
	
	/* Create variables for clock rendering */
	TTF_Font *clockFont = TTF_OpenFont(FontFile, ClockSize);
	SDL_Color clockColor = DefaultClockColor; /* UAH blue */
	SDL_Surface *clockSurface, *timerSurface;
	SDL_Rect clockRect = {0, 0, 0, 0}; /* Clock will be top half of the window when displayed with timer */
	SDL_GetWindowSize(window, &clockRect.w, &clockRect.h);
	clockRect.h /= 2;
	_Bool cPres = 0; /* Track if the timer was presented to the screen */
	
	/* Create variables for timer rendering */
	TTF_Font *timerFont = TTF_OpenFont(FontFile, TimerSize);
	SDL_Color timerColor = DefaultTimerColor; /* UAH Blue */
	SDL_Texture *clockTexture, *timerTexture;
	SDL_Rect timerRect = {0, 0, 0, 0}; /* Timer/stopwatch will be bottom half of the window when displayed with clock */
	SDL_GetWindowSize(window, &timerRect.w, &timerRect.h);
	timerRect.h /= 2;
	timerRect.y = timerRect.h;
	_Bool tPres = 0; /* Track if the timer was presented to the screen */
	
	SDL_Rect fullRect = {0, 0, 0, 0}; /* If only the clock or only the timer/stopwatch is displayed, it will resized to look better on the window*/
	SDL_GetWindowSize(window, &fullRect.w, &fullRect.h);
	fullRect.x = .05*fullRect.w;
	fullRect.y = .25*fullRect.h;
	fullRect.w *= .9;
	fullRect.h *= .5;
	
	/* Setup sound handling */
	SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;
	SDL_AudioDeviceID audioDevice;
	
	_Bool tHours; /* Only display the hours place on the timer if the time remaining >= 60 minutes*/
	_Bool alarmClosed = 1; /* Handler to close the alarm sound after being pkayed */
	_Bool am; /* Display AM/PM on the clock */
	unsigned hms[3]; /* Hours, minutes, and seconds of the clock */
	time_t now; /* Dummy variable to */
	time_t alarmTime;
	
	/* Strings to handle clock and timer/stopwatch displays */
	char clockStr[16];
	char timerStr[16];
	
	/* Start the command interpreter */
	pthread_t cmdThread;
	pthread_create(&cmdThread, NULL, cmdInterpreter, NULL);
	
	/* Main loop */
	while(1){
		if(clockMode){
			/* Get the current time */
			now = time(0) + offset;
			tim = localtime(&now);
			hms[0] = tim->tm_hour;
			hms[1] = tim->tm_min;
			hms[2] = tim->tm_sec;
			
			/* Create the clock string */
			am = 1;
			if(hms[0] == 0){ hms[0] = 12; } /* Make 24-hour format into 12-hour */
			else if(hms[0] > 12){ am = 0; hms[0] -= 12; }
			sprintf(clockStr, "%u:", hms[0]);
			if(hms[1] < 10){ sprintf(clockStr + strlen(clockStr), "0%u", hms[1]); }
			else{ sprintf(clockStr + strlen(clockStr), "%u", hms[1]); }
			if(secs){
				if(hms[2] < 10){ sprintf(clockStr + strlen(clockStr), ":0%u", hms[2]); }
				else{ sprintf(clockStr + strlen(clockStr), ":%u", hms[2]); }
			}
			sprintf(clockStr + strlen(clockStr), " %s", am ? "AM" : "PM");
		}
		
		if(timerMode){
			memset(timerStr, '\0', sizeof(timerStr)); /* Clear the timer/stopwatch string */
			if(stopwatchMode){ /* Stopwatch handling */
				/* Update stopwatch runtime (if unpaused) */
				if(!stopwatchPause){ watchtime = time(0) - initTime; }
				hms[0] = floor((watchtime) / 3600);
				hms[1] = floor((watchtime - hms[0] * 3600) / 60);
				hms[2] = watchtime % 60;
				
				/* Generate the timer/stopwatch string */
				tHours = hms[0] > 0;
				if(tHours){
					if(hms[0] < 10){ sprintf(timerStr, "0%u:", hms[0]); }
					else{ sprintf(timerStr, "%u:", hms[0]); }
				}//else{ sprintf(timerStr, "   "); }
				if(hms[1] < 10){ sprintf(timerStr + strlen(timerStr), "0%u", hms[1]); }
				else{ sprintf(timerStr + strlen(timerStr), "%u", hms[1]); }
				if(hms[2] < 10){ sprintf(timerStr + strlen(timerStr), ":0%u", hms[2]); }
				else{ sprintf(timerStr + strlen(timerStr), ":%u", hms[2]); }
				//if(!tHours){ sprintf(timerStr + strlen(timerStr), "   "); }
			}else{ /* Timer handling */
				/* Update time remaining (if unpaused) */
				if(!timerPause){ timerRem = timerLen - (time(0) - initTime); }
				if(timerRem < 0){ timerRem = 0; }
				hms[0] = floor((timerRem) / 3600);
				hms[1] = floor((timerRem - hms[0] * 3600) / 60);
				hms[2] = timerRem % 60;
				tHours = hms[0] > 0;
				
				/* Generate the timer/stopwatch string */
				if(tHours){
					if(hms[0] < 10){ sprintf(timerStr, "0%u:", hms[0]); }
					else{ sprintf(timerStr, "%u:", hms[0]); }
				}//else{ sprintf(timerStr, "   "); }
				if(hms[1] < 10){ sprintf(timerStr+3, "0%u", hms[1]); }
				else{ sprintf(timerStr+3, "%u", hms[1]); }
				if(hms[2] < 10){ sprintf(timerStr+5, ":0%u", hms[2]); }
				else{ sprintf(timerStr+5, ":%u", hms[2]); }
				//if(!tHours){ sprintf(timerStr+8, "   "); }
				
				/* Play alarm if timer has hit 00:00:00 */
				if(timerRem == 0 && alarmPlay && !timerAlarm){
					timerAlarm = 1;
					system("pactl set-sink-volume @DEFAULT_SINK@ 75%"); /* Make the volume loud enough */
					SDL_LoadWAV(AlarmFile, &wavSpec, &wavBuffer, &wavLength);
					audioDevice = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
					SDL_QueueAudio(audioDevice, wavBuffer, wavLength);
					SDL_PauseAudioDevice(audioDevice, 0);
					alarmTime = time(0); alarmClosed = 0;
				}
			}
		}
		
		/* Close the sound player if alarm time has passed */
		if(!alarmClosed && time(0) - alarmTime >= AlarmLen){
			SDL_CloseAudioDevice(audioDevice);
			alarmClosed = 1;
		}
		
		// Window Events
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT: /* Close the program if a request to close the window is given */
					exit(0);
					break;
			}
		}
		
		/* Clear the window */
		SDL_RenderClear(renderer);
		
		/* Draw the clock */
		if(clockMode){
			clockSurface = TTF_RenderText_Solid(clockFont, clockStr, clockColor);
			clockTexture = SDL_CreateTextureFromSurface(renderer, clockSurface);
			SDL_SetRenderTarget(renderer, clockTexture);
			SDL_RenderCopy(renderer, clockTexture, NULL, timerMode ? &clockRect : &fullRect);
			cPres = 1;
		}else{ cPres = 0; }
		
		/* Draw the timer */
		if(timerMode){
			if(timerRem <= TimerChange && !stopwatchMode){ timerColor = (SDL_Color)AlertTimerColor; }
			else{ timerColor = (SDL_Color)DefaultTimerColor; }
			timerSurface = TTF_RenderText_Solid(timerFont, timerStr, timerColor);
  			timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
			SDL_SetRenderTarget(renderer, timerTexture);
			SDL_RenderCopy(renderer, timerTexture, NULL, clockMode ? &timerRect : &fullRect);
			tPres = 1;
		}else{ tPres = 0; }
		
		/* Update the window */
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		
		/* Clean up the drawn variables */
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

/* Process that reads/interprets serial commands from DMPS */
void *cmdInterpreter(void *vargp){
	int serPort = 16; /* ttyUSB0 */
	char cmdBuff[32]; /* String to recieve commands */
	unsigned bytesRecieved;
	while(1){
		start:
		if(RS232_OpenComport(serPort, BaudRate, RSMode, 0)){ /* 9600 Baud, 8-N-1 */
			goto start;
		}
		SDL_Delay(500); /* Allow time for serial input */
		bytesRecieved = RS232_PollComport(serPort, cmdBuff, 4095); /* Read serial data */
		RS232_CloseComport(serPort);
		if(bytesRecieved > 0){
			cmdBuff[bytesRecieved] = '\0'; /* End the string */
			/* printf("C: %s\n", cmdBuff); */
			if(strncmp(cmdBuff, "setc ", 5) == 0 && strlen(cmdBuff) >= 19){ /* setc command: sets the display time */
				/* Example command: setc 022820232359590000 */
				/*                  setc MMDDYYYYhhmmss0000 */
				cmdBuff[19] = '\0';
				strptime(cmdBuff+5, "%m%d%Y%H%M%S", tim);
				offset = mktime(tim) - time(0);
				
				/* printf("O: %lu\n", mktime(tim)); */
			}else if(strncmp(cmdBuff, "sett ", 5) == 0 && strlen(cmdBuff) >= 13){ /* sett command: sets a timer */
				/* Example command: sett 23:59:59 */
				/*                  setc hh:mm:ss */
				/* Get the timer length */
				timerLen = (cmdBuff[5] - '0') * 36000;
				timerLen += (cmdBuff[6] - '0') * 3600;
				timerLen += (cmdBuff[8] - '0') * 600;
				timerLen += (cmdBuff[9] - '0') * 60;
				timerLen += (cmdBuff[11] - '0') * 10;
				timerLen += (cmdBuff[12] - '0');
				
				/* Start the timer */
				initTime = time(0);
				timerRem = timerLen - (time(0) - initTime);
				stopwatchMode = 0;
				timerMode = 1;
				timerAlarm = 0;
				
				/* printf("T: %u\n", timerLen); */
			}else if(strncmp(cmdBuff, "swon", 4) == 0){ /* swon command: starts a stopwatch */
				/* Start the stopwatch */
				timerMode = 1;
				stopwatchMode = 1;
				initTime = time(0);
				
				/* printf("Stopwatch\n"); */
			}else if(strncmp(cmdBuff, "swoff", 5) == 0){ /* swoff command: turns off the stopwatch (clock will be the only thing display) */
				/* Start the stopwatch */
				timerMode = 0;
				stopwatchMode = 0;
				initTime = time(0);
				
				/* printf("Stopwatch\n"); */
			}else if(strncmp(cmdBuff, "togc", 4) == 0){ /* togc command: toggles the clock on the display */
				clockMode = !clockMode;
				
				/* printf("Clock toggled\n"); */
			}else if(strncmp(cmdBuff, "togt", 4) == 0){ /* togt command: toggles the timer/stopwatch on the display and cancels the timer/stopwatch */
				timerMode = !timerMode;
				if(timerMode){ initTime = time(0); timerLen = DefaultTimer; timerAlarm = 0; }
				
				/* printf("Timer toggled\n"); */
			}else if(strncmp(cmdBuff, "toga", 4) == 0){ /* toga command: toggles the alarm sound */
				alarmPlay = !alarmPlay;
				
				/* printf("Alarm sound toggled\n"); */
			}else if(strncmp(cmdBuff, "pzt", 3) == 0){ /* pzt command: pauses/resumes the timer */
				if(timerMode && !stopwatchMode){
					timerPause = !timerPause;
					if(!timerPause){
						initTime = time(0) - timerLen + timerRem;
						
						/* printf("Timer resumed\n"); */
					}/* else{ printf("Timer paused\n"); } */
				}
			}else if(strncmp(cmdBuff, "pzs", 3) == 0){ /* pzs command: pauses/resumes the stopwatch */
				if(timerMode && stopwatchMode){
					stopwatchPause = !stopwatchPause;
					if(!stopwatchPause){
						initTime = time(0) - watchtime;
						
						/* printf("Timer resumed\n"); */
					}/* else{ printf("Timer paused\n"); } */
				}
			}else if(strncmp(cmdBuff, "sec", 3) == 0){ /* sec command: toggles the seconds on the clock */
					secs = !secs;
					/* printf("Seconds place toggled \n"); */
			}
			memset(cmdBuff, sizeof(cmdBuff), '\0'); /* Clear the command string */
		}
	}
}

