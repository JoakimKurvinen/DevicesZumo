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
#include <math.h>
float conv(float);
float revCheck(float,bool);

/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

#if 1
//battery level//
    
float conv(float input){                            //converts raw sensor input to range 0-1
    float output = (input - 3750) / (24250 - 3750); //practical sensor range was between 3500 and 24500
    if(output > 1){         //makes sure output is between 0-1
        output = 1;
    }
    else if(output < 0){
        output = 0;
    }
    return output;
}

float revCheck(float input, bool isLeft){   //turns >100% slowdown into reverse motor and appropriate slowdown        
    if(input <= 100){                       //e.g. input 115 returns 15 and backwards method on correct motor
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

int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    ADC_Battery_Start(); //Start with Battery check 

    int16 adcresult =0;
    float volts = 0.0;
    
    printf("\nBoot\n");
 
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
    
    struct sensors_ ref;
    reflectance_start();
    reflectance_read(&ref);
        
    int lineCounter = 0;
    float speed = 255;
    bool turningLeft = true;
    bool onBlack = true;    //after getting to initial starting point, robot starts on black
    
    float leftSlow = 0, rightSlow = 0; //slow down speed 
    float leftMult = 0, rightMult = 0; //used only for 2nd method
    float L3=0, L2=0, L1=0, R1=0, R2=0, R3=0; 

    float sumTotal = 0;
    
    //Get to start line
    L3 = conv(ref.l3); //outputs values 0-1
    R3 = conv(ref.r3);
    
    CyDelay(1000);
    PWM_Start();
    
    while(!(R3 > 0.7 && L3 > 0.7)){       //check if outer sensors are over 0.7 threshold (dark)
        BatteryLed_Write(1);
        
        struct sensors_ ref;
        reflectance_start();
        reflectance_read(&ref);
        
        L3 = conv(ref.l3);
        R3 = conv(ref.r3);
        
        PWM_WriteCompare1(90 - (L3 * 80)); //left
        PWM_WriteCompare2(90 - (R3 * 80)); //right
    }
 
    PWM_Stop();
    Systick_Start();
    BatteryLed_Write(0);
    IR_Start();
    //playNote(500, 196.00);
    
    IR_wait(); // wait for IR command
    PWM_Start();
    
    for(;;){  
        
        struct sensors_ ref;
        reflectance_start();
        reflectance_read(&ref);   
        
        L1 = conv(ref.l1);  //practical sensor data ranges from 3500 to 24500 
        R1 = conv(ref.r1);  
        
        L2 = conv(ref.l2);
        R2 = conv(ref.r2);
        
        L3 = conv(ref.l3);
        R3 = conv(ref.r3);
        
        int firstSens = 25;
        int secondSens = 50;
        int thirdSens = 100;
        
        leftSlow = (L1*firstSens + L2*secondSens + L3*thirdSens);
        rightSlow = (R1*firstSens + R2*secondSens + R3*thirdSens);
        
        if(onBlack == false){       //driving code
            //All sensors white - robot is off track and is lost: TURN TOWARDS LAST KNOWN DIRECTION
            if(L3<0.3 && L2<0.3 && L1<0.3 && R1<0.3 && R2<0.3 && R3<0.3){
                if(turningLeft){
                    MotorDirLeft_Write(0);
                    MotorDirRight_Write(1);
                    
                    PWM_WriteCompare1(0); //left    
                    PWM_WriteCompare2(speed); //right    
                }
                else{
                    MotorDirLeft_Write(1);
                    MotorDirRight_Write(0);
                    
                    PWM_WriteCompare1(speed); //left
                    PWM_WriteCompare2(0); //right   
                }
            }
            //Not at start/end or lost: FOLLOW LINE
            else{
                leftMult = (1 - (revCheck(leftSlow,true)/100));     //turn percent slowdown into percent speed
                rightMult = (1 - (revCheck(rightSlow,false)/100));  //e.g. 15% slower => 85% of speed
                
                if(leftMult > rightMult){               //check which value is larger,
                    rightMult = rightMult / leftMult;   //set larger value to 1 and scale smaller one down
                    leftMult = 1;
                }
                else{
                    leftMult = leftMult / rightMult;
                    rightMult = 1;
                }
  
                PWM_WriteCompare1((speed * leftMult)); //left
                PWM_WriteCompare2((speed * rightMult)); //right
                
                //check if robot is currently turning left or right incase it gets off track (for next loop)
                //will only remember left/rightness when robot is on track!
                if(rightMult < leftMult){       
                    turningLeft = true;
                }
                else{
                    turningLeft = false;
                }
            }
        }
         else{           //onBlack is true
            MotorDirLeft_Write(0);
            MotorDirRight_Write(0);
            
            PWM_WriteCompare1(255); //left
            PWM_WriteCompare2(255);
        }
        
        //LINECOUTNING CODE
        if(onBlack){
            if(R3 < 0.3 && L3 < 0.3){   //passing from black to white    
                onBlack = false;
                //BatteryLed_Write(1);
            }
        }
        else{
            if(R3 > 0.7 && L3 > 0.7){   //passing from white to black
                lineCounter++;
                onBlack = true;
                //BatteryLed_Write(0);
            }
        }  
        
        //STOPPING CODE
        if(lineCounter == 2){
            PWM_Stop();
        } 
        //sumTotal += leftMult;
        //sumTotal += rightMult;
    }
}   

#endif


//alternative method (Strategy 2)
#if 0
struct sensors_ ref;
struct sensors_ dig;


void startSensorsAndSetSensorThresholds();
void reachStartLineAndWait();
void passStartLine(uint8 speed);
int calculateProcessVariable();


int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    IR_Start();     
    ADC_Battery_Start();
    
   
    const float fullSpeed = 255;
    const int setPoint = 25;
    float errorBefore = 0;
    int previousPV = 0, count = 0;
    const float Kp = 25; // proportional constant, which helps calculate the proportional part, defined by trial and error
    //Kp =  25(12.6s) 25.5 (13.1s) 20(13.4s) constant and result
        
    const float Kd = 100; //kd = 2.5 (13s) 0.5(13.2) 5.5(13.1)  10.5 50.5(13.2)defined by trial and error, defined after Kp has been finetuned,
    float rightMotorSpeed;
    float leftMotorSpeed;       
        
    bool isOnFirstLine = false; 
    bool isOnStopLine = false;
    bool hasPassedFirstLine = false;
    
    
    //start motors and sensors, and set thresholds
    startSensorsAndSetSensorThresholds();  
  
    
    //start robot until it reaches the start line    
       while(!(ref.l3 == 1 && ref.r3 == 1)){  
            motor_forward(100,10);             
            reflectance_digital(&ref);
            CyDelay(10);
        }        
    motor_forward(0,0);    
    
    
    //wait for IR signal to enter track    
    IR_flush();    
    IR_wait();
    
    
    //Pass the start line
    passStartLine(fullSpeed); 
    
    //Code for Line Following
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
           
        //calculate process variable
        int processVariable = calculateProcessVariable();
        
        if (processVariable == -1) {
            processVariable = previousPV;   //no black line, use value where black line was last seen by sensors
        }else{
            previousPV = processVariable;
        }
        
       
        int error = setPoint - processVariable; 
        //error range : [-25, 25], if error is negative, turn right, if 0, go forward, if positive, turn left    
                
        
        //calculate controller output:
        //the output is the difference in speed between the two motors
        //slowDownSpeed =  proportionalPart + derivativePart; 
        //if error is negative, turn right, left motor goes faster than the right one
        //if error is positive, turn left, right motor goes faster than the left one
        //if error is 0, both motors go at same speed
        
        
        float proportionalPart = Kp * error;         
        
        float derivativePart = (errorBefore - error) * Kd ;       
        
        float slowDownSpeed = proportionalPart + derivativePart;     
        
        
        if(slowDownSpeed == 0){
            rightMotorSpeed = fullSpeed;
            leftMotorSpeed = fullSpeed;
        }else if(slowDownSpeed > 0){
            rightMotorSpeed = fullSpeed;
            leftMotorSpeed = fullSpeed - slowDownSpeed;
        }else if(slowDownSpeed < 0){
            leftMotorSpeed = fullSpeed;
            rightMotorSpeed = fullSpeed + slowDownSpeed;//slow down speed is in negative
        }               
        
        //adjust motors' speed to center robot on black line based on error signal
        motor_turn(leftMotorSpeed,rightMotorSpeed,0);
        errorBefore = error;       
    }
    
    motor_stop();
        
 }


int calculateProcessVariable(){
    reflectance_digital(&ref);
    
    int pv = 0;
    
    if(ref.l3 == 1){
        pv = pv + 10  * 0;
    }
    
    if(ref.l2 == 1){
        pv = pv + 10 * 1;
    }
    
    if(ref.l1 == 1){
        pv = pv + 10 * 2;
    }
    
    if(ref.r1 == 1){
         pv = pv + 10 * 3;
    }
    
    if(ref.r2 == 1){
         pv = pv + 10 * 4;
    }
    
    
    if(ref.r3 == 1){
         pv = pv + 10 * 5;
    }
    
   
    
    int vSum = ref.l1 + ref.l2 + ref.l3 + ref.r1 + ref.r2 + ref.r3;
    
    if (vSum == 0){
        pv = -1;        //if vSum = 0, pv not defined, it may crash the system             
    }else{           
        pv = pv/vSum;     
    } 
        return pv;
}

void startSensorsAndSetSensorThresholds(){ 
    motor_start(); 
    reflectance_start();    
    reflectance_set_threshold(9000, 10000, 11000, 11000, 10000, 9000); // set center sensor threshold to 11000 and others to 9000
    reflectance_digital(&ref);
}


void passStartLine(uint8 speed){     
    motor_forward(speed, 0);   
    reflectance_digital(&ref);
    while(ref.l3 == 1 && ref.r3 == 1){   //keep looping until robot has passed the start line      
    reflectance_digital(&ref);
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


#if 0       //SUMO
//ultrasonic sensor//
struct sensors_ ref;
void startReflectanceAndSetThreshold();
void moveToEntryPoint();
void passEntryLine(uint speed);
void detectEnemy();

    
int scanCap (float input, int threshold){        //capping input at threshold
    if (input > threshold){
        return threshold;    
    }
    else return input;
}

float speedCheck(float speed, bool isleftMotor){
    if (speed < 0){                         //negative speed
        if(isleftMotor){
            MotorDirLeft_Write(1);    
        }
        else MotorDirRight_Write(1);
        
        if (speed < -255) return -255;
        else return -speed;
    }
    else{                                   //pos speed
        if(isleftMotor){
            MotorDirLeft_Write(0);
        }
        else MotorDirRight_Write(0);
        
        if(speed > 255) return 255;
        else return speed;    
    }
}

int scanCheck(int input, bool turnLeft, bool isLeftMotor){  //changes sign if turn direction != corresponding motor
    int buffer;
    buffer = input;
    
    if (!turnLeft) buffer = -buffer;
    
    if (!isLeftMotor) buffer = -buffer;
    
    return buffer;
}

void startReflectanceAndSetThreshold(){    
    reflectance_start();    
    reflectance_set_threshold(16000, 16000, 16000, 16000, 16000, 16000); // set center sensor threshold to 11000 and others to 9000
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
    motor_forward(speed,100);
    reflectance_digital(&ref);
     while(ref.l3 == 1 && ref.r3 ==1 ){                     // pass entry line         
        motor_forward(speed,400);    
        reflectance_digital(&ref);            
     }
    
}
/*
void speedScale(float left, float right){
    if(left > right){               
                    right = right / left;   // scale smaller one down and set larger value to 1
                    left = 1;
    }
    else{
        left = left / right;
        right = 1;
    }
}
*/
int main()
{
    
    CyGlobalIntEnable; 
    UART_1_Start();
    Systick_Start();
    Ultra_Start();                          // Ultra Sonic Start function
    int d = Ultra_GetDistance();            // let threshhold be 30
    bool attackMode = false;
    bool turnLeft = true; //robot starts by turning left so this value is initially set to true
    
    PWM_Start();
    
    float fullSpeed = 255;
    float delayTime = 400;
    
    int threshold = 30; //scan threshold
    //int minThreshold = 5; //sensor value when right up against it, value when robot goes to ramming speed
    int counter = 0;
    float moveSpeed = 120;
    
    float oscSpeed = 180;
    float oscFq = 1800;  //sin(pi / oscFq)
    
    float scanSpeed = 200;
    
    float scanMod = 0; //values from 0 to 1, value is at 1 when lower than threshold-1, 0 otherwise 
    float oscMod = 0;  //values from (-1) to 1, oscilating func, >0 for left, <0 for right
    float moveMod = 1; //makes code more understandable
    
    float leftSpeed = 0;
    float rightSpeed = 0;
    
      
    motor_start();  
    reflectance_start();
    IR_Start();  
    Ultra_Start();
    
    //start reflectance sensors and set threshold 
    startReflectanceAndSetThreshold();
    
    //move forward to entry point
     while(!(ref.l2 == 1 && ref.r2 == 1)){
        motor_forward(100,100);
        reflectance_digital(&ref);
    }
    motor_forward(0,0);
    
    //wait for remore control to enter sumo ring
    IR_flush();
    IR_wait();
    
    
    //pass entry line and enter sumo ring      
    passEntryLine(255);
    reflectance_digital(&ref); 
    
    //somo wrestling code
    for(;;){
        reflectance_digital(&ref); 
        
        if(ref.l3 == 0  && ref.r3 == 0){      //no black, scan   
            if(attackMode != 1){
                d = scanCap(Ultra_GetDistance(), threshold); 
                moveMod = 1;
                if(d <= 5){         //robot is within ramming range, FULL SPEED AHEAD!
                    attackMode = true;
                    PWM_WriteCompare1(speedCheck(255, true));
                    PWM_WriteCompare2(speedCheck(255, false));
                }
                
                else{               //scanning for robots, no ramming
                    
                    if(d < threshold-1){ 
                        scanMod = 1;        //threshold -1
                        moveMod = 5;
                    }
                    else scanMod = 0;
                    
                    oscMod = sin((M_PI /2) +(counter * M_PI / oscFq));
                    
                    counter++;
                    if(counter % (int)oscFq == 0) turnLeft = !turnLeft;        //change bool of turnLeft every oscilation period (when it changes osc direction)
                    
                    leftSpeed = (moveSpeed * moveMod) + (oscSpeed * oscMod) + (scanSpeed * scanCheck(scanMod, turnLeft, true));
                    rightSpeed = (moveSpeed * moveMod) + (oscSpeed * -oscMod) + (scanSpeed * scanCheck(scanMod, turnLeft, false));
                    
                    PWM_WriteCompare1(speedCheck(leftSpeed, true)); //left    
                    PWM_WriteCompare2(speedCheck(rightSpeed, false)); //right 
                }
            }
            else{
                PWM_WriteCompare1(speedCheck(255, true));
                PWM_WriteCompare2(speedCheck(255, false));
            }
        }
        else { //sees black
            attackMode = 0;
            if( ref.l3 == 1){           //l3 on black, retreat first so it will not be out of the ring and turn 
                
                motor_backward(fullSpeed,100);            
                motor_turn(0,fullSpeed,delayTime);   
                
            }else  if(ref.r3 == 1){         //r3 on black, retreat first so it will not be out of the ring and turn  
                
                motor_backward(fullSpeed,100); 
                motor_turn(fullSpeed,0,delayTime); 
                
            }else if( ref.l3 == 1 && ref.r3 == 1){ //both outer sensors on black, 
               BatteryLed_Write(1); // Switch led off 
               MotorDirLeft_Write(0);                   
               MotorDirRight_Write(1);  
               PWM_WriteCompare1(fullSpeed); 
               PWM_WriteCompare2(fullSpeed);  
               CyDelay(delayTime);
            }
        }
    }
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
