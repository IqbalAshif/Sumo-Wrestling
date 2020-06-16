#include <project.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
#include <stdlib.h>

#define BLACK 1
#define WHITE 0


#if 1
void run_to (int color)		//Color (black or white) funtion
{
  struct sensors_ dig;
  bool run = true;
  while (run)
    {
      reflectance_digital (&dig);
      vTaskDelay (100);
      if (dig.l3 == color && dig.r3 == color)
	{
	  motor_forward (0, 0);	// set speed to zero to stop motors
	  run = false;
	}
      else
	{
	  motor_forward (50, 50);	// moving forward
	}
    }
}


void zmain (void)
{
  TickType_t start = 0;
  TickType_t end = 0;
  xTaskGetTickCount ();		//start timer since boot
  printf ("\nBoot\n");
  send_mqtt ("Zumo03/boot", "zumo");	//At boot

  Ultra_Start ();		// Ultra Sonic Start function
  IR_Start ();
  struct sensors_ dig;
  vTaskDelay (100);

  while (SW1_Read ())		//SW1_Read() read SW1 on pSoC board
  vTaskDelay (10);
  reflectance_start ();     //refelection funtion start
  reflectance_set_threshold (9000, 9000, 11000, 11000, 9000, 9000);	// set center sensor threshold to 11000 and others to 9000
  motor_start ();		// enable motor controller
  motor_forward (0, 0);		// set speed to zero to stop motors
  run_to (BLACK);		//goes to top of the cicle circumference
  IR_flush ();			// clear IR receive buffer
  send_mqtt ("Zumo03/ready", "zumo");

  IR_wait ();			//waits for IR command
  start = xTaskGetTickCount ();	//time at giving IR command
  print_mqtt ("Zumo03/start", "%d", start);
  run_to (WHITE);		//goes into the circle   

  bool run = true;
  while (run)
    {
      if (SW1_Read () == 0)         //If the start button is pressed
	{
	  motor_forward (0, 0);
	  run = false;
	  end = xTaskGetTickCount ();
	  print_mqtt ("Zumo03/stop", "%d", end);
	  print_mqtt ("Zumo03/time", "%d", end - start);
	}
      //Wrestling is on
      else
	{
	  reflectance_digital (&dig);
	  //when going to the edge of the ring
	  if (dig.l3 == 1 || dig.r3 == 1 || dig.l2 == 1 || dig.r2 == 1
	      || dig.l1 == 1 || dig.r1 == 1)
	    {
	      motor_forward (0, 0);
	      vTaskDelay (500);
	      motor_backward (30, 1000);
	      vTaskDelay (500);

	      int count = rand () % 2;

	      if (count == 0)
		{
		  motor_turn (50, 0, 1000);
		}
	      else
		{
		  motor_turn (50, 0, 1500);
		}
	    }
	  //When running inside the ring and avoiding the obstacle
	  else
	    {
	      int d = Ultra_GetDistance ();
	      if (d >= 11)
		{
		  motor_forward (50, 100);
		}
	      else
		{
		  motor_forward (0, 0);
		  vTaskDelay (500);
		  motor_backward (50, 200);
		  vTaskDelay (500);
		  int count = rand () % 2;   

		  if (count == 0)
		    {
		      motor_turn (95, 0, 500);
		      print_mqtt ("Zumo03/obstacle", "%d", xTaskGetTickCount ());
		    }
		  else
		    {
		      motor_turn (0, 95, 500);
		      print_mqtt ("Zumo03/obstacle", "%d", xTaskGetTickCount ());
		    }
		}
	    }
	}
    }
  while (true)

    {
      vTaskDelay (100);
    }
}
#endif
