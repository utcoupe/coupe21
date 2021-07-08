/****************************************
 * Author : Quentin C			*
 * Mail : quentin.chateau@gmail.com	*
 * Date : 29/11/13			*
 ****************************************/
/*
DIG1 and DIG2 defined to 0 :
linear adjustment of the speed

might be configured to smth else in order use speed control
see datasheet of DEC-MODULE-24/2
*/
#include "shared_asserv/parameters.h"
#include "motor.h"
#include "pwm.h"

const uint8_t NO_PWM = 127;
extern Pwm g_right_pwm;
extern Pwm g_left_pwm;
//Controleur :
//-255:0   : Marche arriere
//0:255 : Marche avant

void setPWM(int motor_side, int pwm) {
	if(pwm > 255)
		pwm = 255;
	else if(pwm < -255)
		pwm = -255;

	static int last_pwms[2] = {NO_PWM, NO_PWM};
	int& last_pwm = last_pwms[motor_side -1];

	if (pwm != last_pwm) {
	    // Brushless status must change
        last_pwm = pwm;
		
		// From -255-255 to 0-255
		pwm = NO_PWM + pwm / 2.0;

        if (motor_side == MOTOR_LEFT) {
			g_left_pwm.set_duty_cycle(pwm);
        } else {
			g_right_pwm.set_duty_cycle(pwm);
        }
    }
}

void motorsInit(void) {
	//TODO: gérer les pins de brake
	HAL_GPIO_WritePin(MOT_L_BRAKE_GPIO_Port,MOT_L_BRAKE_Pin,HIGH);
	HAL_GPIO_WritePin(MOT_R_BRAKE_GPIO_Port,MOT_R_BRAKE_Pin,HIGH);

	//TODO: avoir un PWM externe
	g_right_pwm.set_timer_freq(32000);
	g_left_pwm.set_timer_freq(32000);
	g_right_pwm.set_duty_cycle(NO_PWM);
	g_left_pwm.set_duty_cycle(NO_PWM);

	// SPD is the new DIR
	HAL_GPIO_WritePin(MOT_L_DIR_GPIO_Port,MOT_L_DIR_Pin,HIGH);
	HAL_GPIO_WritePin(MOT_R_DIR_GPIO_Port,MOT_R_DIR_Pin,HIGH);
}

void setPWMLeft(int pwm){
	//les moteurs sont faces à face, pour avancer 
	//il faut qu'il tournent dans un sens différent
	pwm = -pwm;
	setPWM(MOTOR_LEFT, pwm);
}

void setPWMRight(int pwm){
	setPWM(MOTOR_RIGHT, pwm);
}

