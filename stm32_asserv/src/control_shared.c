/**
 * Author : Paul Constant
 * Date : 22/12/29
 * 
 * This file is used in both stm32_asserv and asserv_simu.py.
 * It regroups all functions linked to the command law of the robot.
 * 
 * A shared library is generated from this file and imported in the python 
 * code of the simulation. This way, the stm32 command law can be tested without 
 * the robots.
 * The file is imported like any other file in stm32_asserv.
 *
 * Be careful when editing : 
 * 
 * - If you change a function definition (name, parameters, return value),
 *   update the simulation asserv so that it does not break.
 * 
 * - If you change the name of this file, make the change in the compile.sh script as well.
**/

#include "control_shared.h"
#include "parameters.h"
#include "robotstate_shared.h"
#include "local_math.h"
#include <math.h>

int controlPos(float dd, float da) {
	int ret;
	float dda, ddd, max_speed;
	float ang_spd, lin_spd;

	int pos_error;
	float ddd_final, dda_next;

	if (FifoGetGoal(FifoCurrentIndex()+1)->type == TYPE_POS) {
		float da_next, dd_final;
		int dx, dy, x, y, goal_count;

		// Calculate angle between current and next goal
		goal_t *current_goal = FifoCurrentGoal();
		goal_t *next_goal = FifoGetGoal(FifoCurrentIndex()+1);

		x = current_goal->data.pos_data.x;
		y = current_goal->data.pos_data.y;
		dx = next_goal->data.pos_data.x - x;
		dy = next_goal->data.pos_data.y - y;
		da_next = atan2f(dy, dx) - current_pos.angle;
		
		// Calculate distance to final goal
		dd_final = 0;
		goal_count = 1;
		while (FifoGetGoal(FifoCurrentIndex()+goal_count)->type == TYPE_POS) {
			next_goal = FifoGetGoal(FifoCurrentIndex()+goal_count++);

			dx = next_goal->data.pos_data.x - x;
			dy = next_goal->data.pos_data.y - y;

			dd_final += sqrtf(powf(dx, 2.0) + powf(dy, 2.0));

			x = next_goal->data.pos_data.x;
			y = next_goal->data.pos_data.y;
		}

		// Adjust direction
		if (sign(dd) == -1) {
			da_next = da_next + (float)(M_PI);
			dd_final = -dd_final;
		}
		da_next = moduloTwoPI(da_next);

		pos_error = ERROR_INTERMEDIATE_POS;
		dda_next = da_next * (float)(ENTRAXE_ENC / 2.0);

		if (da_next > (float)MAX_ANGLE_DIFF)
			ddd_final = 0;
		else
			ddd_final = dd_final * expf(-fabsf(K_DISTANCE_REDUCTION * da_next));
	}
	else {
		pos_error = ERROR_POS;
		dda_next = 0;
		ddd_final = 0;
	}

	dda = da * (float)(ENTRAXE_ENC / 2.0);
	if (da > (float)MAX_ANGLE_DIFF) 
		ddd = 0;
	else
		ddd = dd * expf(-fabsf(K_DISTANCE_REDUCTION * da));

	max_speed = control.max_spd;
	if (control.status_bits & SLOWGO_BIT) {
		max_speed *= (float)EMERGENCY_SLOW_GO_RATIO;
	}

	ang_spd = control.speeds.angular_speed;
	lin_spd = control.speeds.linear_speed;

	control.speeds.angular_speed = calcSpeed(ang_spd, dda, 
			max_speed * control.rot_spd_ratio, dda_next);
	control.speeds.linear_speed = calcSpeed(lin_spd, ddd,
			max_speed, ddd_final);

	ret = 0;
	if (fabsf(dd) < pos_error) {
		ret |= POS_REACHED;
	}
	if (fabsf(da) < (float)ERROR_ANGLE) {
		ret |= ANG_REACHED;
	}

	return ret;
}

/* INTERNAL FUNCTIONS */

void goalAngle(goal_t *goal) {
	float angle, da;
	angle = goal->data.ang_data.angle;
	da = angle - current_pos.angle;

	if (goal->data.ang_data.modulo) {
		da = moduloTwoPI(da);
	}
	
	if (controlPos(0, da) & ANG_REACHED) {
		goal->is_reached = 1;
	}
	applyPID();
}

void goalPos(goal_t *goal) {
	int x, y;
	float dx, dy, da, dd, goal_a;

	x = goal->data.pos_data.x;
	y = goal->data.pos_data.y;
	dx = x - current_pos.x;
	dy = y - current_pos.y;
	goal_a = atan2f(dy, dx);
	da = (goal_a - current_pos.angle);
	da = moduloTwoPI(da);
	dd = sqrtf(powf(dx, 2.0)+powf(dy, 2.0));

	if (goal->data.pos_data.d == ANY) {
		if (fabsf(da) > (float)(CONE_ALIGNEMENT)) {
			da = moduloPI(da);
			dd = - dd;
		}
	} else if (goal->data.pos_data.d == BACKWARD) {
		dd = - dd;
		da = moduloTwoPI(da + (float)(M_PI));
	}

	if (controlPos(dd, da) & POS_REACHED) {
		goal->is_reached = 1;
	}
	applyPID();
}

float calcSpeed(float init_spd, float dd, float max_spd, float final_speed) {
	float dd_abs, acc_spd, dec_spd, target_spd;
	int d_sign;
	dd_abs = fabsf(dd);
	d_sign = sign(dd);

	init_spd *= d_sign;
	acc_spd = init_spd + control.max_acc * (float)DT;
	dec_spd = sqrtf(powf(final_speed, 2) + 2*control.max_acc*dd_abs);
	target_spd = fminf(max_spd, fminf(acc_spd, dec_spd))*d_sign;
	return target_spd;
}

void stopRobot(void) {
	float speed;

    speed = fabsf(control.speeds.angular_speed);
    if (BRK_COEFF != 0.0) {
        speed -= control.max_acc * (float)(DT * BRK_COEFF);
    } else {
        speed = 0.0;
    }
    speed = fmaxf(0.0f, speed);
    control.speeds.angular_speed = speed;

    speed = fabsf(control.speeds.linear_speed);
    if (BRK_COEFF != 0.0) {
        speed -= control.max_acc * (float)(DT * BRK_COEFF);
    } else {
        speed = 0.0;
    }
    speed = fmaxf(0.0f, speed);
    control.speeds.linear_speed = sign(control.speeds.linear_speed)*speed;

	if (fabsf(wheels_spd.left) + fabsf(wheels_spd.right) < (float)SPD_TO_STOP) {
		allStop();
	} else {
		applyPID();
	}
}

void allStop(void) {
	control.speeds.pwm_left = 0;
	control.speeds.pwm_right = 0;
	control.speeds.linear_speed = 0;
	control.speeds.angular_speed = 0;
}

void applyPID(void) {
	float left_spd, right_spd;
	float left_ds, right_ds;
	left_spd = control.speeds.linear_speed - control.speeds.angular_speed;
	right_spd = control.speeds.linear_speed + control.speeds.angular_speed;
	left_ds = left_spd - wheels_spd.left;
	right_ds = right_spd - wheels_spd.right;
	// Control feed forward
    //control.speeds.pwm_left = speedToPwm(left_spd) + PIDCompute(&PID_left, left_ds);
    //control.speeds.pwm_right = speedToPwm(right_spd) + PIDCompute(&PID_right, right_ds);
	control.speeds.pwm_left = (int)ceilf(PIDCompute(&PID_left, left_ds));
    control.speeds.pwm_right = (int)ceilf(PIDCompute(&PID_right, right_ds));
}