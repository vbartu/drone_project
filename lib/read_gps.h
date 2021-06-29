


#ifndef PATMOS_READ_GPS_H
#define PATMOS_READ_GPS_H


#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "lib/read_gps.h"
#include "sensors/gps.h"
#include <machine/rtc.h>

#define error_print false

#define UART2 ((volatile _IODEV unsigned *)PATMOS_IO_UART2)
#define UART3 ((volatile _IODEV unsigned *)PATMOS_IO_UART3)

void millis(int milliseconds)
{
  unsigned int timer_ms = (get_cpu_usecs() / 1000);
  unsigned int loop_timer = timer_ms;
  while (timer_ms - loop_timer < milliseconds)timer_ms = (get_cpu_usecs() / 1000);
}

int uart2_write(unsigned char data)
{
  if ((*UART2 & 0x00000001) != 0)
  {
    *UART2 = (unsigned int)data;
    return 1;
  }
  else
  {
    data = 0;
    return 0;
  }
}

int uart2_read(unsigned char* data)
{
  if ((*UART2 & 0x00000002) != 0)
  {
    *data = (unsigned char)(*(UART2 + 1) & 0x000000FF);
    return 1;
  }
  else
  {
    *data = 0;
    return 0;
  }
}

int uart3_write(unsigned char data)
{
  if ((*UART3 & 0x00000001) != 0)
  {
    *UART3 = (unsigned int)data;
    return 1;
  }
  else
  {
    data = 0;
    return 0;
  }
}

int uart3_read(unsigned char* data)
{
  if ((*UART3 & 0x00000002) != 0)
  {
    *data = (unsigned char)(*(UART3 + 1) & 0x000000FF);
    return 1;
  }
  else
  {
    *data = 0;
    return 0;
  }
}

bool read_gps(struct gps_tpv * tpv) {
    unsigned char gps_data=0;
    int result;
    //---- to filter Data
    char str_c[500];
    char cRMC[6] = "$GNRMC";
    char cVTG[6] = "$GNVTG";
    char cGSA[6] = "$GNGSA";
// Drone GPS = GNRMC, NEO6M GPS = GPRMC  
    int loop_counter = 0;
    //------ to filter data ------------------
    int str_temp[6], str_mode = 0;
    uint8_t tmpMode = 0;
    char str_tempc[6], str_tempMode[10];
    int start_temp = 0, start_mode = 0;
    int end_temp = 6, end_mode = 10;
    bool b_temp = false;
    bool printed = false;
    bool equal_RMC = false;
    int start_c = 6;
    int end_c = 300;
    bool equal_VTG = false, equal_GSA = false;


    // millis(250);

    for(loop_counter=0;loop_counter<1000;loop_counter++) {     
        millis(4);                                                    //Stay in this loop until the data variable data holds a q.
            while (uart2_read(&gps_data)) {
                //printf("%c",gps_data);
                //The delimiter "$" is 36 in ASCII
                if (gps_data == 36) {
                    b_temp = true;
                }
                if (b_temp && (start_temp < end_temp)) {
                    str_temp[start_temp] = gps_data;
                    str_tempc[start_temp] = (char) gps_data;
                    start_temp++;
                }

                if (equal_RMC && (start_c < end_c)) {
                    if ((char) gps_data != '\0'){
                    str_c[start_c] = (char) gps_data;
                    start_c++;
                    }
                }
                //find the RMC string
                if ((start_temp == end_temp) && !equal_RMC) {
                    b_temp = false;
                    int comp = 0;
                    for (int j = 0; j < 6; j++) {
                        comp = comp + str_tempc[j] - cRMC[j];
                        str_c[j] = str_tempc[j];
                    }
                    if (comp == 0) {
                        equal_RMC = true;
                    }
                    start_temp = 0; // Try again
                }

                //find the VTG string
                if ((start_temp == end_temp) && equal_RMC && !equal_VTG) {
                    b_temp = false;
                    int comp = 0;
                    for (int j = 0; j < 6; j++) {
                        comp = comp + str_tempc[j] - cVTG[j];
                    }
                    if (comp == 0) {
                        equal_VTG = true;
                          //printf("\n\n");
                    } else {
                        equal_RMC = false; //If the next string is not VTG, it must go back false
                        start_c = 6;
                    }
                    start_temp = 0; // Try again?
                }

                //find the GSA string
                if ((start_temp == end_temp) && equal_RMC && !equal_GSA) {
                    b_temp = false;
                    int comp = 0;
                    for (int j = 0; j < 6; j++) {
                        comp = comp + str_tempc[j] - cGSA[j];
                        str_tempMode[j] = str_tempc[j];
                        str_mode++;
                    }
                    if (comp == 0) {
                        equal_GSA = true;
                    }else{
                        str_mode = 0;
                    }
                    start_temp = 0; // Try again
                } 
                else if (equal_GSA && (str_mode<10))
                {
                     str_tempMode[str_mode] = (char) gps_data;
                     str_mode++;
                }else if (equal_GSA && (str_mode == 10) && (tmpMode == 0))
                {
                    tmpMode = (int)str_tempMode[9] - '0';
                }
            }
            if( (start_c >= end_c)){
                break;
            }
    }
    //printf("%s",str_c);

    result = gps_decode(tpv, str_c);
    if (result != GPS_OK && error_print)
    {
        printf("gps error");
    }
    // Overwrite value of mode with the tmp one
    if(equal_GSA && (str_mode == 10)){
        tpv->mode = tmpMode;
    }
    return equal_RMC;
}


#endif //PATMOS_READ_GPS_H
