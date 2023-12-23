#define WinWidth 0 /* Window will be fulscreen */
#define WinHeight 0
#define WinName "Charger Time" /* Name of window X11 reads */

#define FontFile "/home/josh/School/ETLC/countdown-display/fonts/Nunito/NunitoSans-VariableFont_YTLC,opsz,wdth,wght.ttf" /* Location of the font file */
#define AlarmFile "/home/josh/School/ETLC/countdown-display/sounds/chime.wav" /* Location of the alarm sound */
#define EEFile "/home/josh/School/ETLC/countdown-display/sounds/ee.wav" /* Location of the alarm sound */
#define AlarmLen 6 /* Length to play alarm before closing (seconds) */

#define RendererFlags SDL_RENDERER_PRESENTVSYNC
#define WindowFlags SDL_WINDOW_FULLSCREEN_DESKTOP /* The window will always go fullscreen */

#define ClockSize 600 /* Clock's font size */
#define TimerSize 750 /* Timer's font size */

#define DefaultTimerLen 120

/* Default clock font color: #0077C8 */
#define DefaultClockR 0x00
#define DefaultClockG 0x77
#define DefaultClockB 0xC8
#define DefaultClockA 0xFF
#define DefaultClockColor {DefaultClockR, DefaultClockG, DefaultClockB, DefaultClockA}

/* Default timer font color: #0077C8 */
#define DefaultTimerR 0
#define DefaultTimerG 119
#define DefaultTimerB 200
#define DefaultTimerA 255
#define DefaultTimerColor {DefaultTimerR, DefaultTimerG, DefaultTimerB, DefaultTimerA}

/* Timer alert font color: #FF2222 */
#define AlertTimerR 0xFF
#define AlertTimerG 0x33
#define AlertTimerB 0x33
#define AlertTimerA 255
#define AlertTimerColor {AlertTimerR, AlertTimerG, AlertTimerB, AlertTimerA}

#define TimerChange 60 /* Benchmark time to change the timer's font */

#define BaudRate 9600 /* Serial baud rate */
#define RSMode "8N1" /* Serial parameter*/
