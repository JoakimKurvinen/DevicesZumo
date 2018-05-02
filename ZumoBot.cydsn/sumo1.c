
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
int rread(void);
struct sensors_ ref;



void startReflectanceAndSetThreshold();
void moveToEntryPoint();
void passEntryLine(uint speed);
void detectEnemy();



/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

#if 1
//battery level//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
       
    ADC_Battery_Start();        
    int16 adcresult =0;
    float volts = 0.0;
   
    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    
    float fullSpeed = 255;
    float delayTime = 400;
    int range = 20;
    static volatile int distance;
    
    motor_start();  
    reflectance_start();
    IR_Start();  
    Ultra_Start();
    
    
    startReflectanceAndSetThreshold();
    
    //moveToEntryPoint();
     while(!(ref.l2 == 1 && ref.r2 == 1)){
        motor_forward(100,100);
        reflectance_digital(&ref);
    }
    motor_forward(0,0);
    
    //wait for remore control to enter sumo ring
    IR_flush();
    IR_wait();
    
    
    //pass entry line and enter sumo ring
      
    passEntryLine(fullSpeed);
    
        
    for(;;){
        reflectance_digital(&ref);        
//        Ultra_Start();
//        distance = Ultra_GetDistance(); //Ultra sonic sensors to detect distance to other robots
//        CyDelay(0);  
               
       
         //all sensors see white, inside sumo ring
    if(ref.l3 == 0  && ref.r3 == 0){            
        
        //use Ultra sonic sensors to detect enemy
        
        Ultra_Start();
        distance = Ultra_GetDistance(); 
        CyDelay(1);  
        
        if(distance > range  ){     // if enemy not found, keep turning to search for it  
            motor_turn(fullSpeed*0.7,0,0); //turning with only one motor moving
//           MotorDirLeft_Write(0);          //or: turning with both motors moviing, one forward, one backward         
//           MotorDirRight_Write(1);  
//           PWM_WriteCompare1(fullSpeed*0.4); 
//           PWM_WriteCompare2(fullSpeed*0.4);
          // CyDelay(delayTime); 
        }

        else{                        // if enemy is found within range, go at full speed towards it           
           motor_forward(fullSpeed,0);                   
        }
     
     reflectance_digital(&ref);  //keep updating sensor readings
    }
    
    // sensor sees black, robot reaches border, turn back and move forward    
    else{  
        if( ref.l3 == 1){           //l3 on black, retreat first so it will not be out of the ring and turn 

            motor_backward(fullSpeed,100);            
            motor_turn(0,fullSpeed,delayTime);            
           
        }else  if(ref.r3 == 1){         //r3 on black, retreat first so it will not be out of the ring and turn                     
             motor_backward(fullSpeed,100); 
             motor_turn(fullSpeed,0,delayTime);              
        
        }else if( ref.l3 == 1 && ref.r3 == 1){ //both outer sensors on black, 
           
           MotorDirLeft_Write(0);                   
           MotorDirRight_Write(1);  
           PWM_WriteCompare1(fullSpeed); 
           PWM_WriteCompare2(fullSpeed);  
           CyDelay(delayTime);
        }
    reflectance_digital(&ref);
    }
    
  } 




void startReflectanceAndSetThreshold(){    
    reflectance_start();    
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    reflectance_digital(&ref);
}

void moveToEntryPoint(){
    while(!(ref.l3 == 1 && ref.r3 == 1)){
        motor_forward(100,100);
        reflectance_digital(&ref);
    }
    motor_forward(0,0);
}

void passEntryLine(uint speed ){
    motor_forward(speed,400);
    reflectance_digital(&ref);
     while(ref.l3 == 1 && ref.r3 ==1 ){                     // pass entry line         
        motor_forward(speed,400);    
        reflectance_digital(&ref);            
     }
    motor_forward(speed, 350);
}

void detectEnemy(){
        Ultra_Start();
        ultrasonic_handler();
        Ultra_GetDistance();  

}