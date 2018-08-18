// dhq August 2018
// Hackster Contest
#include "mbed.h"
#include <math.h>
#include <stdlib.h>

#include "SerialGPS.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "MAX32620FTHR.h"
#include "MAX77650.h"
#include "MAX17055.h"

#include "GPSimg.h"
#include "BatteryImg.h"

 
#define POWER_HOLD_OFF 0 
#define POWER_HOLD_ON 1 

// Configure GPIO voltage
MAX32620FTHR max32620fthr(MAX32620FTHR::VIO_3V3);

#define BACKGROUND  BLACK
#define FOREGROUND  YELLOW
#define DELAY 1000

I2C i2c(I2C2_SDA, I2C2_SCL);
DigitalOut  myled(LED1);
SerialGPS   gps(P3_1, P3_0,9600);

MAX17055 max17055(i2c);             // Fuel gauge
MAX77650 max77650(i2c);             // MAX77650 PMIC and Charger

// Configure PMIC to drive green LED
static const char ledGreen[] = {
    MAX77650::CNFG_LED0_A,
    0x04, 0x44, 0x04,
    0x0F, 0x0F, 0x0F,
    0x01,
};

DigitalOut pw_Hold(P2_2, POWER_HOLD_ON); 
//SDFileSystem sd(P0_5, P0_6, P0_4, P1_3, "sd"); // the pinout on the mbed Cool Components workshop board


int main() {
    
    // Configure TFT display
    LCD_SCAN_DIR LCD_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
    LCD_Init(LCD_ScanDir ); 
    
    wait_ms(1000);
    
    LCD_Clear(BLACK);
    GUI_DrawRectangle(0,0,156,127,RED,DRAW_EMPTY,DOT_PIXEL_2X2);
    myled = !myled; // toggle a led

    char sats[16] = {};
    char longitude[16] = {};
    char latitude[16] = {};
    char geoid[16] = {};
    char altitude[16] = {};
    char time[16] = {};  
    
    char voltage[16] = {};   
    char avgVoltage[16] = {};  
    char current[16] = {};  
                    
    ///int status;
    float f_value;
    
    // Configure fuel gauge
    // Set sense resistor value
    max17055.init(0.05f);

    // Turn off Low-Dropout Linear Regulator (LDO); unused on MAX32620FTHR platform
    max77650.disableLDO();
    
    // Set SIMO Buck-Boost Regulator 2 target voltage; provides VDDIOH
    max77650.setSBB2Voltage(3.3f);

    // Turn PMIC green LED on, red and blue off
    max77650.writeReg(ledGreen, sizeof(ledGreen));
    

    while(1) {

        wait(0.5f); // wait a small period of time
        
        if (gps.sample()) {
            myled = myled ? 0 : 1;
            //pc.printf("sats %d, long %f, lat %f, alt %f, geoid %f, time %f\n\r", gps.sats, gps.longitude, gps.latitude, gps.alt, gps.geoid, gps.time);
            
            sprintf(sats, "SAT: %d ", gps.sats);
            sprintf(longitude, "LONG: %2.2f ", gps.longitude);
            sprintf(latitude, "LAT: %2.2f ", gps.latitude);
            sprintf(altitude, "ALT: %2.2f ", gps.alt);
            sprintf(geoid, "GEOID: %2.2f ", gps.geoid);
            sprintf(time, "TIME: %2.2f ", gps.time);

            LCD_Clear(BLACK);
            GUI_DisplayImage(50,10,gps_lock,50,50);
            GUI_DrawRectangle(0,0,156,127,YELLOW,DRAW_EMPTY,DOT_PIXEL_2X2);

            GUI_DisString_EN(10,80,sats,&Font12,GUI_BACKGROUND,YELLOW);
            GUI_DisString_EN(10,90,longitude,&Font12,GUI_BACKGROUND,YELLOW);
            GUI_DisString_EN(10,100,latitude,&Font12,GUI_BACKGROUND,YELLOW);
            GUI_DisString_EN(10,110,altitude,&Font12,GUI_BACKGROUND,CYAN);
            //GUI_DisString_EN(10,90,geoid,&Font12,GUI_BACKGROUND,CYAN);
            //GUI_DisString_EN(10,110,time,&Font12,GUI_BACKGROUND,CYAN);
            
            wait(2);
            LCD_Clear(BLACK);
            
            // Print formatted voltage, and battery current 
            max17055.v_cell(&f_value);
            sprintf(voltage,"V=%6.2fV", f_value);
            //max17055.avg_v_cell(&f_value);
            //sprintf(avgVoltage,"Vavg = %6.3f", f_value);   
            max17055.current(&f_value);
            sprintf(current,"I=%6.2fmA", f_value);
            //max17055.avg_current(&f_value);
           
            GUI_DisplayImage(50,10,batt_full,50,50);    
            GUI_DisString_EN(10,70,voltage,&Font20,GUI_BACKGROUND,YELLOW);
            //GUI_DisString_EN(10,90,avgVoltage,&Font12,GUI_BACKGROUND,YELLOW);
            GUI_DisString_EN(10,100,current,&Font20,GUI_BACKGROUND,CYAN);
            
            wait(2);


        }    
        myled = !myled; // toggle a led
    }
}


