/**
 * ?????????????
 * ??PI??????????????PTC??
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <ptc_duty_control.h>


float LimitDutyCycle(float duty) {
    if (duty > DUTY_CYCLE_MAX) return DUTY_CYCLE_MAX;
    if (duty < DUTY_CYCLE_MIN) return DUTY_CYCLE_MIN;
    return duty;
}

void PTC_Controller_Init(void) {
    ctrl->target_power = 0.0f;
    ctrl->current_power = 0.0f;
    ctrl->output_duty = 0.0f;
    
    ctrl->base_kp = 0.000006f; //0.000006f;   /* ?????0.00001f */
    ctrl->base_ki =0.00000006f;//0.00000006f   /* ?????0.0000008f */
    ctrl->base_kd = 0.0006f;     /* ?????0.0002f */
     
    ctrl->kp = ctrl->base_kp;
    ctrl->ki = ctrl->base_ki;
    ctrl->kd = ctrl->base_kd;
    
    ctrl->integral = 0.0f;
    ctrl->last_error = 0.0f;
    ctrl->is_enabled = false;
    ctrl->is_stable = false;
    ctrl->stable_counter = 0;
    
    ctrl->max_integral = 0.001f;       
    ctrl->max_duty_change = 0.03f;     
    ctrl->output_limit = DUTY_CYCLE_MAX;
    
    ctrl->max_error = 0.0f;
    ctrl->avg_error = 0.0f;
    ctrl->control_cycles = 0;
    
    ctrl->adaptive_kp = ctrl->kp;
    ctrl->adaptive_ki = ctrl->ki;
    ctrl->last_duty_output = 0.0f;
    
}

void PTC_Controller_SetTarget(float target_power) 
{
    if (target_power >= MIN_POWER_LIMIT) 
    {
        ctrl->is_enabled = true;

        ctrl->target_power = target_power+50;
    } 
    else 
    {
        ctrl->is_enabled = false;
        ctrl->output_duty = 0.0f;
        ctrl->integral = 0.0f;
    }
}

uint16_t PTC_Controller_Update(uint8_t runCmd) {

  if(runCmd==0)return 0;
    if (ctrl->target_power < MIN_POWER_LIMIT) 
    {
        ctrl->output_duty = 0.0f;
        ctrl->integral = 0.0f;
        ctrl->is_stable = false;
        return 0;
    }
    ctrl->current_power = (float)debug_info.PTC_RealPower;
    float error = ctrl->target_power - ctrl->current_power;
    float abs_error = fabs(error);
    
    ctrl->control_cycles++;
    if (abs_error > ctrl->max_error) {
        ctrl->max_error = abs_error;
    }
    ctrl->avg_error = (ctrl->avg_error * (ctrl->control_cycles - 1) + abs_error) 
                      / ctrl->control_cycles;
    
    /* ========== PID ========== */
    
    float p_term = ctrl->kp * error;
     
    float integral_increment = ctrl->ki * error;
    float new_integral = ctrl->integral + integral_increment;
    
    if (new_integral > ctrl->max_integral) {
        new_integral = ctrl->max_integral;
    } else if (new_integral < -ctrl->max_integral) {
        new_integral = -ctrl->max_integral;
    }
    
    float error_diff = error - ctrl->last_error;
    float d_term = ctrl->kd * error_diff;
    
    ctrl->last_error = error;
    
    float predicted_duty = ctrl->output_duty + p_term + new_integral + d_term;
    bool allow_integration = true;
    
    if (predicted_duty >= DUTY_CYCLE_MAX || predicted_duty <= DUTY_CYCLE_MIN) {
        allow_integration = false; 
    }
    
    if (allow_integration) {
        ctrl->integral = new_integral;
    } else {
        if ((predicted_duty >= DUTY_CYCLE_MAX && error > 0) ||
            (predicted_duty <= DUTY_CYCLE_MIN && error < 0)) {
            ctrl->integral = new_integral * 0.3f; 
        }
    }
    
    float duty_change = p_term + ctrl->integral + d_term;
//   debug_info.testData1=p_term*10000;
//     debug_info.testData=ctrl->integral*10000;
    float actual_max_change = ctrl->max_duty_change;
    if (ctrl->is_stable) {
        actual_max_change *= 0.2f; 
        
        if (abs_error < POWER_ERROR_THRESHOLD * 0.5f) {
            duty_change = 0;
        }
    }
 
    if (duty_change > actual_max_change) {
        duty_change = actual_max_change;
    } else if (duty_change < -actual_max_change) {
        duty_change = -actual_max_change;
    }
    
    float new_duty = ctrl->output_duty + duty_change;
    new_duty = LimitDutyCycle(new_duty);
    
    if (new_duty >= DUTY_CYCLE_MAX || new_duty <= DUTY_CYCLE_MIN) {
        ctrl->integral *= 0.95f; 
    }
    
    ctrl->output_duty = new_duty;
    
  
    if (abs_error < POWER_ERROR_THRESHOLD) 
    {
            if (!ctrl->is_stable)
            {
                ctrl->is_stable = true;
            }
    } 
    else
    {
        ctrl->stable_counter++;
        uint32_t required_cycles =10;
        if (ctrl->stable_counter >= required_cycles)
        {
            ctrl->stable_counter = 0;
            ctrl->is_stable = false;
        }
    }
    
    ctrl->last_duty_output = ctrl->output_duty;

    return (uint16_t)(ctrl->output_duty * 100);
}

void PTC_Controller_Reset(void)
{ 
    ctrl->output_duty = 0.0f;
    ctrl->integral = 0.0f;
    ctrl->last_error = 0.0f;
    ctrl->is_enabled = false;
    ctrl->is_stable = false;
    ctrl->stable_counter = 0;
    ctrl->max_error = 0.0f;
    ctrl->avg_error = 0.0f;
    ctrl->control_cycles = 0;
    
}
void ptc_duty_control_init()
{
     PTC_Controller_Init();
}
void PWM_DUTY(void)
{
    static uint16_t g_pwmCounter = 0;
    static uint16_t dutyPoint_temp=0;
    static  bool toggleFlag=0;
    static bool PTC_Stater_Flag=0;
    static uint8_t delayGegAD=0;
    static uint16_t g_ad_thick_film_current_filter=0;
        debug_info.PTC_Running_flag=1;
        g_pwmCounter++;
        // ??PWM??????????
        if(g_pwmCounter >= PWM_PERIOD_TICKS)
        {
            g_pwmCounter = 0;
        }
        if(g_pwm_thick_film <= 0)
        {
            THICK_PWM(0);
            debug_info.PtcPeakCurrents=0;//WPTC ��ֵ����
            debug_info.PtcCurrents=0;//WPTC ��Ч����
            debug_info.PTC_RealPower=0;
        }
        if((g_pwmCounter == (PWM_PERIOD_TICKS-g_pwm_thick_film))&&(toggleFlag)) 
        {
            THICK_PWM(1);
            toggleFlag=0;
            debug_info.toggleFlag=0;
            
            g_ad_thick_film_current_filter = g_ad_thick_film_current_filter - (g_ad_thick_film_current_filter>>3) + (g_ad_thick_film_current>>3);
            debug_info.PtcPeakCurrents=ad_to_current(g_ad_thick_film_current_filter*1.55);//WPTC ��ֵ��
            debug_info.PtcCurrents=debug_info.PtcPeakCurrents*g_pwm_thick_film/100;//WPTC ��Ч����
            uint32_t PTC_RealPower_file=(uint32_t)(debug_info.pVdcValue*((float)debug_info.PtcCurrents/10));
            debug_info.PTC_RealPower=(((uint32_t)debug_info.PTC_RealPower*62258 + (uint32_t)PTC_RealPower_file*3278)>>16 );
            
            debug_info.PtcPeakCurrents_fast=ad_to_current(g_ad_thick_film_current*1.55);//WPTC ��ֵ��
//            if(debug_info.PtcPeakCurrents_fast>0)
//            debug_info.PTC_Resistance = (uint32_t)debug_info.pVdcValue*10 / debug_info.PtcPeakCurrents_fast;
            
        } 
        else if((g_pwmCounter == 0)&&(!toggleFlag)) 
        {
            THICK_PWM(0);
            toggleFlag=1;
            debug_info.toggleFlag=1;
        }
}
void PWM_DUTY_TEST(void)
{
  static  bool toggleFlag=0;
   if(toggleFlag == 0)
        {
          toggleFlag=1;
            THICK_PWM(0);
        }
   else
   {
     toggleFlag=0;
     THICK_PWM(1);
   }
     
}


float P_MAX  =5000.0f,HEAT__R=16.5 ;  

float ThickFilm_Heat_Ctrl(float duty_set, float v_bus)
{
    // 1. �����޷�
    if(duty_set < 0.0f)  duty_set = 0.0f;
    if(duty_set >= 99.0f) duty_set = 99.0f;
    
    // ��ѹ�޷�
    if(v_bus < Cal_DiagDcBusUvTrip_V_u16) v_bus =Cal_DiagDcBusUvTrip_V_u16 ;
    if(v_bus > Cal_DiagDcBusOvTrip_V_u16) v_bus = Cal_DiagDcBusOvTrip_V_u16;

    // 2. ��λ��ռ�ձ�ӳ��Ŀ��㹦�� 0~7000W
    float p_target = duty_set / 100.0f * P_MAX;

    // 3. ���㵱ǰ��ѹ�£�Ҫ�ﵽĿ�깦����Ҫ��PWMռ�ձ�
    // D = (P * R) / U^2
   // if(duty_set<60)HEAT__R=23;
   // else if(duty_set<80)HEAT__R=24;
   // else HEAT__R=25;
/*       if (duty_set <= 60) {
            HEAT__R = 22;
        } else if (duty_set >= 100) {
            HEAT__R = 23.5;
        } else {
            HEAT__R = 22 + (float)(duty_set - 60) * 2.0f / 40.0f;
        } */
       float duty_out = (p_target * HEAT__R) / (v_bus * v_bus);

    // ת�ٷֱ�
    duty_out *= 100.0f;

    // 4. ����޷�����
    if(duty_out < 2.0f)   duty_out = 0.0f;
    if(duty_out > 100.0f) duty_out = 100.0f;

    return duty_out;
}
