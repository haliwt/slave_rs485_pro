#include "bsp_control.h"
#include "bsp.h"

uint8_t interval_time_stop_run ;
uint8_t gFan_continueRun ;


void (* fan_continue_run)(void);

static void Current_Works_State(void);
static void Works_Rest_Cycle_TenMinutes(void);
static void Fan_Run_Fun(void);
//static void Fan_Stop(void);
static void Fan_RunTenMinutes(void);
static void CompareValue_Temperature(void);


/*****************************************************************
	*
	*Function Name:static void Ultrasonic_PWM_OnOff(uint8_t onoff)
	*Function:
	*
	*
	*
*****************************************************************/
void bsp_Init(void)
{
    
	Fan_Continue_RunTenMinutes(Fan_RunTenMinutes);


}

/*****************************************************************
	*
	*Function Name:void Mainboard_Run_Process_Handler(void)
	*Function:run main board of function that ptc ,plasma,ultrasonic.
	*         continue 2 hours and have a rest ten minutes.
	*Input Ref:NO
	*Return Ref:NO
	*
*****************************************************************/
void Mainboard_Run_Process_Handler(void)
{

  static uint8_t power_off_flag;
   switch(g_tMain.rs485_Command_label){


    case power_on:
		power_off_flag=0;
        g_tMain.gPower_On = power_on;
		interval_time_stop_run =0;
        g_tMain.gTimer_continuce_works_time=0;
	    g_tMain.gPtc = 1;
		g_tMain.gPlasma=1;
		g_tMain.gUltrasonic =1;
		Fan_Run_Fun();
		g_tMain.ptc_warning =0;
		g_tMain.fan_warning = 0;
		g_tMain.gTimer_sensor_detect_times = 66;
		g_tModS.rs485_send_answering_signal_flag = 0;
		Buzzer_KeySound();
	    g_tMain.rs485_Command_label= run_update_data;
    break;


	case run_update_data:
		switch(interval_time_stop_run){

         case 0 :
			Current_Works_State();
	    
		break;

		case 1:
			    
			Works_Rest_Cycle_TenMinutes();
		break;

   		}


	break;


	case power_off:
        if(power_off_flag==0){
			power_off_flag++;
		g_tMain.gPower_On = power_off;
	    interval_time_stop_run =0;
        g_tMain.gTimer_continuce_works_time=0;
		gFan_continueRun=1;
	    g_tMain.gPtc = 0;
		g_tMain.gPlasma=0;
		g_tMain.gUltrasonic =0;
		PTC_IO_SetLow();
		PLASMA_IO_SetLow();
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);//ultrasnoic off
		Buzzer_KeySound();

        }
        fan_continue_run();


	break;

	case power_dc_power_on:

	    PTC_IO_SetLow();
		PLASMA_IO_SetLow();
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);//ultrasnoic off
		Fan_Stop();


	break;

  }
}
/*****************************************************************
	  *
	  *Function Name:void Mainboard_Run_Process_Handler(void)
	  *Function
	  *Input Ref:NO 		
	  *Return Ref:NO
	  *
*****************************************************************/
static void Current_Works_State(void)
{
     static uint8_t cycle_run;

     switch(cycle_run){

	     case 0:
            if(g_tMain.gTimer_sensor_detect_times > 63){
			  g_tMain.gTimer_sensor_detect_times=0;
	          Update_DHT11_Value_Handler();

            }
	        cycle_run =1;
		 	
		 break;

		 case 1:

             if(g_tMain.gTimer_fan_works_times > 32){
			 	g_tMain.gTimer_fan_works_times =0;
				if(g_tMain.gPtc ==1 || g_tMain.gPlasma ==1 || g_tMain.gUltrasonic ==1){

					Fan_Run_Fun();

	             }
				 else if(g_tMain.gPtc ==0 &&  g_tMain.gPlasma ==0 && g_tMain.gUltrasonic ==0){
					Fan_Stop();
				 }

             }
		 	
		 	 if(g_tMain.gPtc == 1){
			    PTC_IO_SetHigh(); 

			 }
			 else{
				PTC_IO_SetLow();

			 }
		  cycle_run =2;

		 break;


		 case 2:
		 	if(g_tMain.gPlasma ==1){

              PLASMA_IO_SetHigh();
			}
			else{

			  PLASMA_IO_SetLow();
			}
		 	
          cycle_run =3;
		 break;


		 case 3:

		  if(g_tMain.gUltrasonic ==1){

			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);//ultrasnoic ON 
		  }
		  else{

           HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);//ultrasnoic off
		  }

         cycle_run =4;
		 break;

		 case 4:

		   if(g_tMain.gTimer_ptc_adc_times > 5){
		      g_tMain.gTimer_ptc_adc_times =0;

		     switch(g_tMain.ptc_warning ){

			 case 0:
	
                
             	Get_PTC_Temperature_Voltage(ADC_CHANNEL_1,5);
			 	Judge_PTC_Temperature_Value();
			   
			 

		     
			 break;

			 case 1: //default be happed 
			 	
			    if( g_tModS.rs485_send_answering_signal_flag == 0){

				    
				         MODS_SendHostError_Signal(0xB0);
					     Buzzer_Ptc_Error_Sound();

                      
			    }
				else{

					g_tMain.ptc_warning ++;

				}

			 break;

			 case 2: //buzzer sound 
                   Buzzer_Ptc_Error_Sound();

			 break;
			
		     }

		   }
		  cycle_run =5;

		 break;

		 case 5:

		   if(g_tMain.gTimer_fan_adc_times > 8){

		      g_tMain.gTimer_fan_adc_times =0;

		      switch(g_tMain.fan_warning ){

			  case 0:
			  
				Get_Fan_Adc_Fun(ADC_CHANNEL_0,5);
				  
	          
			  break;

			  case 1:
			
                 if( g_tModS.rs485_send_answering_signal_flag == 0){
				  	
				       MODS_SendHostError_Signal(0xA0);
					   Buzzer_Fan_Error_Sound();
                 	
				 }
				 else {
				    g_tMain.fan_warning ++;


				  }

			  break;

			  case 2: //buzzer sound
			  	
                   Buzzer_Fan_Error_Sound();

			  break;

		     }
			  

		   }
		 

		 cycle_run =6;

		 break;

		 case 6: //set up temperature value and compare sensor value 
         
		 CompareValue_Temperature();

		 cycle_run =7;

		 break;

		 case 7:

			if(g_tMain.gTimer_continuce_works_time > 7200){
		     g_tMain.gTimer_continuce_works_time =0;
	         interval_time_stop_run =1;
		     gFan_continueRun =1;
			 g_tMain.gTimer_fan_counter=0;
		    }

			cycle_run =0;
		 break;

   }

}
/*****************************************************************
	  *
	  *Function Name:static void Works_Rest_Cycle_TenMinutes()
	  *Function
	  *Input Ref:NO 		
	  *Return Ref:NO
	  *
*****************************************************************/
static void Works_Rest_Cycle_TenMinutes(void)
{
	if(g_tMain.gTimer_continuce_works_time < 10){
		PLASMA_IO_SetLow(); //
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);//ultrasnoic Off 
		PTC_IO_SetLow(); 
		
    }

	fan_continue_run();

	

	if(g_tMain.gTimer_continuce_works_time > 600){
		g_tMain.gTimer_continuce_works_time=0;
		interval_time_stop_run =0;

    }

}



static void Fan_Run_Fun(void)
{
    FAN_IO_RUN_SetHigh();
	FAN_IO_SetLow();

}

void Fan_Stop(void)
{

	FAN_IO_RUN_SetLow();
	FAN_IO_SetLow();


}

/*****************************************************************
	  *
	  *Function Name:static void Fan_RunTenMinutes(void)
	  *Function: function of pointer by special run function
	  *Input Ref:NO 		
	  *Return Ref:NO
	  *
*****************************************************************/
static void Fan_RunTenMinutes(void)
{

	if(gFan_continueRun ==1){

		if(g_tMain.gTimer_fan_counter < 60){

		Fan_Run_Fun();
		}       

		if(g_tMain.gTimer_fan_counter > 59){

		g_tMain.gTimer_fan_counter=0;

		gFan_continueRun++;
		Fan_Stop();
		}

	}
	else Fan_Stop();


}

//call for  function of pointer
void Fan_Continue_RunTenMinutes(void (*fan_run)(void))
{

    fan_continue_run = fan_run;

}


/*************************************************************
*
	 *Function Name:static void Fan_RunTenMinutes(void)
	 *Function: function of pointer by special run function
	 *Input Ref:NO		   
	 *Return Ref:NO
	 *

**************************************************************/
static void CompareValue_Temperature(void)
{
	static uint8_t set_temperature_value;

	if(g_tMain.gTimer_compare_temp > 60){
		g_tMain.gTimer_compare_temp =0;
        if(g_tMain.read_temperature_value[0] >19 && g_tMain.read_temperature_value[0] < 41){
               
		 
		  
		if( g_tMain.gTemperature >= g_tMain.read_temperature_value[0] || g_tMain.gTemperature >40){//envirment temperature
	  
				g_tMain.gPtc = 0;
                
		       
			    
	   }
	   else if((g_tMain.read_temperature_value[0] -3) >= g_tMain.gTemperature){


           

                g_tMain.gPtc = 1;
        
		  }
	  
	    
	  }
     else{ //no define set up temperature value 
		if(g_tMain.gTemperature  >40 ){//envirment temperature
			
			g_tMain.gPtc = 0;
		

		}
        else if(g_tMain.gTemperature  < 39){
			
			  g_tMain.gPtc = 1;
             
          }
			    
      }
	}

}


