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

/* [] END OF FILE */


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
   bool isOnFirstLine = false; 
    bool isOnStopLine = false;
    bool hasPassedFirstLine = false;
    /*
    while(isOnStopLine == false){   
        
        //check if robot is on stop line
        reflectance_digital(&ref);     
        if(ref.l3 == 1 && ref.r3 == 1){  //is robot on black line?     
            
                    if(isOnFirstLine == false && hasPassedFirstLine == false){  // is robot on the first black line for the first time?
                        isOnFirstLine = true;     
                    }else if( isOnFirstLine == true && hasPassedFirstLine == false){
                        //robot is still on the first black line, do nothing
                    }else if(hasPassedFirstLine == true){ // robot is on the stop line
                        isOnStopLine = true;
                    }
            
        }else{ // robot not on black line 
                    if(isOnFirstLine == true){   //robot has passed the first line
                        hasPassedFirstLine = true;
                        isOnFirstLine = false;
                    }
        }
        
    }*/