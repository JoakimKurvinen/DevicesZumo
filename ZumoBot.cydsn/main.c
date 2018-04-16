/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

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
#include <stdbool.h>
int rread(void);
float conv(float);
float revCheck(float,bool);
float speedCheck(float);
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

#if 1
//battery level//
    
float conv(float input){                            //converts raw sensor input to range 0-1
    float output = (input - 3500) / (24500 - 3500); //practical sensor range was between 3500 and 24500
    return output;
}

float revCheck(float input, bool isLeft){   //turns >100% slowdown into reverse motor and appropriate slowdown        
    if(input <= 100){                       // input 115 returns 15 and backwards on correct motor
        if(isLeft == true){
            MotorDirLeft_Write(0);
        }
        else{
            MotorDirRight_Write(0);
        }
        return input;
    }
    else{
        if(isLeft == true){
            MotorDirLeft_Write(1);
        }
        else{
            MotorDirRight_Write(1);
        }
        return ((input - 100));
    }
}

float speedCheck(float input){ //returns input with cap at 255
    if(input > 255){
        return 255;   
    }
    else{
    return input;   
    }
}

int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    
    ADC_Battery_Start(); //Start with Battery check 

    int16 adcresult =0;
    float volts = 0.0;
    
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    
    ADC_Battery_StartConvert();
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            volts = (float) adcresult / 4095 * 5 * (3/2);
            
            // Print both ADC results and converted value
            printf("%d %f\r\n",adcresult, volts);
            /*
            if(volts < 4){          //Battery Check, sound can be annoying
                        playNote(600, 164.81); 
                        playNote(300, 196.00);
                        playNote(600, 164.81);
            }
            */
        }
    
    //Battery check ends here, BEGIN LINE COMMANDS   
    PWM_Start();
    bool running = true;
    int lineCounter = 1;
    int timer = 0;
    float speed = 255;
    bool turningLeft = true;
    Systick_Start();
    for(;;) 
    {  
        if(SW1_Read() == 0){ //start running when button is pressed
            running = true;
            PWM_Start();
            PWM_WriteCompare1(255); //left
            MotorDirLeft_Write(0);
            PWM_WriteCompare2(255); //right
            MotorDirRight_Write(0);
            CyDelay(250);
        }
        while(running == true){
                
            struct sensors_ ref;
            reflectance_start();
            reflectance_read(&ref);
            
            float leftSlow = 0, rightSlow = 0;
            float leftMult = 0, rightMult = 0; //used only for 2nd method
            float L3=0, L2=0, L1=0, R1=0, R2=0, R3=0;   
            
            L1 = ref.l1;  //practical sensor data ranges from 3500 to 24500 
            R1 = ref.r1;  
            
            L2 = ref.l2;
            R2 = ref.r2;
            
            L3 = ref.l3;
            R3 = ref.r3;
            
            //1st Method
            leftSlow = (conv(L1)*25 + conv(L2)*60 + conv(L3)*120); //Output values will be 0 to 1 after conv() method
            rightSlow = (conv(R1)*25 + conv(R2)*60 + conv(R3)*120);//spd 320
                
            //1st sensor, 25%, 60%, 130%, spd 310       13.38
            //1st sensor, 15%, 2nd, 50%, 3rd 100% speed 290     14.00
            /*
            //2nd method
            leftSlow = (conv(L1)*25 + conv(L2)*60 + conv(L3)*120); //Output values will be 0 to 1 after conv() method
            rightSlow = (conv(R1)*25 + conv(R2)*60 + conv(R3)*120);
            
            if(leftSlow > rightSlow){               //outputs range 0-1 where one motor will always be at one and 
                leftMult = (1 - (revCheck(rightSlow,false)/100)) / (1 - (revCheck(leftSlow,true)/100));    //other will be proportionally smaller
                rightMult = 1;
            }
            else{
                rightMult = (1 - (revCheck(leftSlow,true)/100)) / (1 - (revCheck(rightSlow,false)/100));
                leftMult = 1;
            }
            
            printf("L3 %f L2 %f L1 %f R1 %f R2 %f R3 %f\n", conv(L3), conv(L2), conv(L1), conv(R1), conv(R2), conv(R3));
            printf("%f, %f\n", leftMult, rightMult);
            */
            if(conv(R3) > 0.7 && conv(L3) > 0.7){ //L3,R3 sensors are black: full stop
                lineCounter++;
                if(lineCounter % 2 == 0){
                    running = false; 
                    PWM_WriteCompare1(0); //left
                    PWM_WriteCompare2(0); //right
                    PWM_Stop();
                }
                /*timer = timer/100;
                int j = 0;
                
                for(int i=0; i<timer; i++){
                    float k = (float) pow(1.059463094359, j);
                    playNote(300, 164.81 * k);
                    j++;
                    j = j % 12;
                }
                timer = 0;
                CyDelay(3000);
                playNote(300, 196.00);
                playNote(300, 164.81); 
                CyDelay(10000);*/
            }
            //all sensors white - robot is off track and is lost. turn towards 
            else if(conv(L3)<0.3 && conv(L2)<0.3 && conv(L1)<0.3 && conv(R1)<0.3 && conv(R2)<0.3 && conv(R3)<0.3){
                if(turningLeft){
                    MotorDirLeft_Write(0);
                    MotorDirRight_Write(1);
                    
                    PWM_WriteCompare1(speed); //left    //NEED SPEEDCHECKS HERE IF USING METHOD 1
                    PWM_WriteCompare2(speed); //right    
                }
                else{
                    MotorDirLeft_Write(1);
                    MotorDirRight_Write(0);
                    
                    PWM_WriteCompare1(speed); //left
                    PWM_WriteCompare2(speed); //right   
                }
            }
            else{                       //not at start/end or lost, follow line                               
                //method1
                //PWM_WriteCompare1(speedCheck(speed * (1 - (revCheck(leftSlow,true)/100)))); //left
                //PWM_WriteCompare2(speedCheck(speed * (1 - (revCheck(rightSlow,false)/100)))); //right
                
                //method2
                
                leftMult = (1 - (revCheck(leftSlow,true)/100)); //check if *speed is needed
                rightMult = (1 - (revCheck(rightSlow,false)/100));
                
                if(leftMult > rightMult){              
                    rightMult = rightMult / leftMult;
                    leftMult = 1;
                }
                else{
                    leftMult = leftMult / rightMult;
                    rightMult = 1;
                }
  
                PWM_WriteCompare1((speed * leftMult)); //left
                PWM_WriteCompare2((speed * rightMult)); //right
                
                //check if robot is currently turning left or right incase it gets off track (for next cycle)
                //will only remember left/rightness when robot is on track
                if(leftSlow > rightSlow){       
                    turningLeft = true;
                }
                else{
                    turningLeft = false;
                }
            }
            
            
        }    
    }
}   

#endif

#if 0
// button
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    bool led = false;
    
    for(;;)
    {
        // toggle led state when button is pressed
        if(SW1_Read() == 0) {
            led = !led;
            BatteryLed_Write(led);
            ShieldLed_Write(led);
            if(led) printf("Led is ON\n");
            else printf("Led is OFF\n");
            Beep(1000, 150);
            while(SW1_Read() == 0) CyDelay(10); // wait while button is being pressed
        }        
    }
 }   
#endif


#if 0
//ultrasonic sensor//
int main()
{
    /*
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    Ultra_Start();                          // Ultra Sonic Start function
    while(1) {
        int d = Ultra_GetDistance();
        //If you want to print out the value  
        printf("distance = %d\r\n", d);
        CyDelay(200);    
    }
    */
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    
    printf("testtest\n");
    
    for(;;){
        float tempArray[50];
        char musicSheet[] = "C4, D4, E4, F4, G4, A4, B4, C5";
        playSong(musicSheet, tempArray);
        printf("%f, %f, %f, testtestTESTTEST\n", tempArray[0],tempArray[1],tempArray[2]);
        playNote(300, 196.00);
        playNote(300, tempArray[0]);
        setTempo(300, tempArray);
        
    CyDelay(2000);
    }
    printf("TestTwo");
}   
#endif


#if 0
//IR receiver//
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    IR_Start();
    
    uint32_t IR_val; 
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    IR_wait(); // wait for IR command
    printf("IR command received\n");
    
    // print received IR pulses and their lengths
    for(;;)
    {
        if(IR_get(&IR_val)) {
            int l = IR_val & IR_SIGNAL_MASK; // get pulse length
            int b = 0;
            if((IR_val & IR_SIGNAL_HIGH) != 0) b = 1; // get pulse state (0/1)
            printf("%d %d\r\n",b, l);
            //printf("%d %lu\r\n",IR_val & IR_SIGNAL_HIGH ? 1 : 0, (unsigned long) (IR_val & IR_SIGNAL_MASK));
        }
    }    
 }   
#endif


#if 0
//reflectance//
int main()
{
    struct sensors_ ref;
    struct sensors_ dig;

    Systick_Start();

    CyGlobalIntEnable; 
    UART_1_Start();
  
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    

    for(;;)
    {
        // read raw sensor values
        reflectance_read(&ref);
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       // print out each period of reflectance sensors
        
        // read digital values that are based on threshold. 0 = white, 1 = black
        // when blackness value is over threshold the sensors reads 1, otherwise 0
        reflectance_digital(&dig);      //print out 0 or 1 according to results of reflectance period
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);        //print out 0 or 1 according to results of reflectance period
        
        CyDelay(200);
    }
}   
#endif


#if 0
//motor//
int main() //TEST TRACK
{
    CyGlobalIntEnable; 
    UART_1_Start();

    float motorDiff = -7;
    float speed = 100;
    float curveConst = -50;
    
    CyDelay(3700);
    
    Notes(300, 164.81);
    Notes(600, 196.00);
    
    CyDelay(500);
    
    PWM_Start();
    
    //First Straight
    MotorDirLeft_Write(0);      // set LeftMotor forward mode
    MotorDirRight_Write(0);     // set RightMotor forward mode
    
    PWM_WriteCompare1(speed); //left
    PWM_WriteCompare2(speed + motorDiff); //right
    CyDelay(3600);
    
    //First Turn (Right)
    PWM_WriteCompare1(speed);  //turning right
    PWM_WriteCompare2(0);
    CyDelay(1470);
    
    //Second Straight
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed + motorDiff);
    CyDelay(2900);
    
    //Second Turn
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(0);
    CyDelay(1570);
    
    //Third Straight
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed + motorDiff);
    CyDelay(3000);
    
    //Third Turn
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(0);
    CyDelay(1900);
    
    //Fourth Curve
    PWM_WriteCompare1(speed); 
    PWM_WriteCompare2(speed + motorDiff + curveConst);
    CyDelay(5500);
    
    PWM_Stop();
    
    Notes(300, 196.00);
    Notes(600, 164.81);
    
    for(;;)
    {

    }
}
#endif


/* [] END OF FILE */
