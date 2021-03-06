/****************************************
 * Author : Quentin C			*
 * Mail : quentin.chateau@gmail.com	*
 * Date : 29/11/13			*
 ****************************************/
#ifndef PARAMETERS_H
#define PARAMETERS_H

#define BAUDRATE 57600
#define SERIAL_TYPE SERIAL_8N1
#define ARDUINO_ID "ard_asserv"
#define DEBUG_TARGET_SPEED 0
#define DEBUG_MAINLOOP 0
//todo add an enum with the different debug levels available
#define DEBUG_LEVEL 2

/* Simple ou Double ou Quadruple evaluation ? 
 * La quadruple evaluation utilise 4 interruption par tick
 * (une pour chaque changement de valeur des fils A et B)
 *
 * La double evaluation utilise 2 interruptions par tick
 * (une pour chaque changement de valeur du fil A
 *
 * La simple evaluation utilise 1 interruption par tick
 * (front montant du fil A)
 *
 * Ces trois méthodes sont equivalentes
 * La quadruple evaluation est plus précise mais utilise
 * plus de puissance processeur
 * Il convient de tester la plus adapté selon le processeur
 * et le nombre de ticks du codeur 
 * 
 * OPTIONS : '1' - '2 - '4' */

#define ENCODER_EVAL 1

#define USE_SHARP 0
#define EMERGENCY_STOP_DISTANCE 0.3 // m

#define HZ 100
#define DT (1.0/HZ)
#define AUTO_STATUS_HZ 10 // must be a divider a HZ or 0 to disable

#define SPD_MAX 1000 //mm/s 1000
#define ACC_MAX 1500 //mm/s2
#define RATIO_ROT_SPD_MAX 0.6
#define K_DISTANCE_REDUCTION 20 // réduction de la vitesse linéaire quand on tourne
#define EMERGENCY_WAIT_TIME 30 // seconds
#define EMERGENCY_SLOW_GO_RATIO 0.3 // spd = 0.3*max_spd in slow_go mode

#define BLOCK_TIME 5000 // ms - time between each block check
#define BLOCK_TIME_AUTO_STOP 500 //time in ms for PWM to stop
#define BLOCK_MIN_DIST 5 // mm - distance to move to consider we moved

#define ENC_RESOLUTION 1024 //resolution du codeur

// 31,24 and 31,38 initial
//31,05 and 31,20
#define ENC_LEFT_RADIUS 31.40 //REGLE PAR TEST - rayon de la roue codeuse (31.38 origin 31.15 good)
#define ENC_RIGHT_RADIUS 31.58 //REGLE PAR TEST - rayon de la roue codeuse (31.24)
#define ENTRAXE_ENC 201.35 //REGLE PAR TES - Distance entre chaque roue codeuse en mm (202.37), base 200,8

#define ERROR_ANGLE 0.030 //erreur en angle(radians) maximale pour considérer l'objectif comme atteint
#define ERROR_POS 5 // erreur en position (mm)  maximale pour considérer l'objectif comme atteint
#define SPD_TO_STOP 10

#define CONE_ALIGNEMENT (M_PI/2.0) // 100 = NEVER

#define PID_P 0.26 //0.25
#define PID_I 250.0 //130
#define PID_D 0.5 //13
#define PID_BIAS 0
#define PID_I_MAX 150
#define PID_OUT_MAX 255

#define BRK_COEFF 3.0

// Control feed-forward, pwm = a*spd + b
#define SPD_TO_PWM_A 0.15
#define SPD_TO_PWM_B 5

#define LEFT_P (PID_P)
#define LEFT_I (PID_I)
#define LEFT_D (PID_D)
#define LEFT_BIAS (PID_BIAS)

#define RIGHT_P (PID_P)
#define RIGHT_I (PID_I)
#define RIGHT_D (PID_D)
#define RIGHT_BIAS (PID_BIAS)

#define PID_I_RATIO (1/1000.0)
#define PID_D_RATIO (1/1000.0)

#define TIME_BETWEEN_ORDERS 0 // s
#define KEEP_LAST_GOAL 0

//DEFINES ARDUINO
#define SERIAL_MAIN Serial

#endif
