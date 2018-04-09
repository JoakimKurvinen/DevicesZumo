/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <stdbool.h>
#include <project.h>
#include <stdio.h>
#include "Systick.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "I2C_made.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "IR.h"
#include "Ambient.h"
#include "Beep.h"
#include <time.h>
#include <sys/time.h>
#include "music.h"

//char notes[] = "A4, E4, D4";

void Beep(uint32, uint8);          //load beep function
//void Notes(uint32, float);
int tempo = 300;
int delay = 0;

/*
float * playSong (char musicInput[]){
    float musicOutput[50];
    int semiT;
    char mi;
    char temp[4];
    int i = 0;      //keep track of letters 
    int j= 0;       //keep track of words
    mi = musicInput[i];
    while (mi != '\0'){
        while (mi != ','){
            musicInput[i] = temp[i];
            i++;
            mi = musicInput[i];
        }
        semiT = getNote(temp);
        musicOutput[j] = 440 * (2^(1/12))^semiT;
        j++;
    }
    return musicOutput;
}
*/
int getNote (char input[]){     //gets input such as E4, and returns
    char nt;                    //amount of semitones away from A4
    int i = 0;
    int result = 0;
    while (nt != '\n')
    {
        nt = input[i];
        switch(nt){
        case 'C':
            result += -9;
            break;
        case 'c':
            result += -8;
            break;
        case 'D':
            result += -7;
            break;
        case 'd':
            result += -6;
            break;
        case 'E':
            result += -5;
            break;
        case 'F':
            result += -4;
            break;
        case 'f':
            result += -3;
            break;
        case 'G':
            result += -2;
            break;
        case 'g':
            result += -1;
            break;
        case 'A':
            result += 0;
            break;
        case 'a':
            result += 1;
            break;
        case 'B':
            result += 2;
            break;
        case '4':
            result += 0;
            break;
        case '5':
            result += 12;
            break;
        case '6':
            result += 12 * 2;
            break;
        case '7':
            result += 12 * 3;
            break;
        case '3':
            result += 12 * -1;
            break;
        case '2':
            result += 12 * -2;
            break;
        case '1':
            result += 12 * -3;
            break;
        default:
            result += 0;
            break;
        }
        i++;
    } 
    return result;
}


void Notes(uint32 length, float HzNote)
{            
    uint16 pitch = 200000 / HzNote;
    uint16 cmp = pitch / 2;
    Buzzer_PWM_Start();
    Buzzer_PWM_WriteCompare(cmp);
    Buzzer_PWM_WritePeriod(pitch);
    CyDelay(length);
    Buzzer_PWM_Stop();
} 

void E3 (uint32 length)         //E3 = 164.81
{
    length = length*tempo - delay;
    Notes(length, 164.81);
    CyDelay(delay);
}

void D3 (uint32 length)         //D3 = 146.83
{
    length = length*tempo - delay;
    Notes(length, 146.83);
    CyDelay(delay);
}

void C3 (uint32 length)         //C3 = 130.81
{
    length = length*tempo - delay;
    Notes(length, 130.81);
    CyDelay(delay);
}

void G3 (uint32 length)         //G3 = 196.00
{
    length = length*tempo - delay;
    Notes(length, 196.00);
    CyDelay(delay);
}



/* [] END OF FILE */
