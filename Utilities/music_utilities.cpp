#undef __ARM_FP

#include "mbed.h"
#include "music.h"


// notes in the melody:
int melody[] = {NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A3, NOTE_B3, NOTE_C4};
 
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {4, 4, 4, 4, 4, 4, 4, 4};
// notes in the melody:
int melody2[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_GS3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
 
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations2[] = {4, 8, 8, 4, 4, 4, 4, 4};
//Set Port A, GPIO Pin 1 (PA_1) as digitial out put pin
//declare a buzzer as a DigitalOut object by passing PA_1 as the initialition parameter 
DigitalOut buzzerMusic(PA_1); 

void tone(int freq, int duration) 
{
    int k;
    long period, count;

    duration = 1000/duration;
    if (freq == 0) 
        thread_sleep_for(duration); // rest
    else 
    {
        period = 1000000 / (long) freq;
        period = period / 2;


        count = (long) duration * 1000 / period;

        for (k = 0; k < count; k++) //Determines duration of tone
        {
            wait_us(period);
            buzzerMusic = !buzzerMusic; //Invert logic level at RC0
        }
        thread_sleep_for(5);
    }
    return;
}


void music(void) 
{
   int i;

    buzzerMusic = 0;
	
    for(i=0; i<8; i++) 
         tone(melody[i],noteDurations[i]);
            
    return;
}

