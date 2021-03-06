#include "parameters.h"
#include "block.h"
#include "robotstate.h"
#include "goals.h"
#include "compat.h"


void ComputeIsBlocked(void) {
#if BLOCK_TIME
	static int last_goal_nr = -1;
	static long last_time = 0;
	static pos_t last_pos = {0, 0, 0, 0};
	long now;
	float dist;
	int block_time = BLOCK_TIME; 
	goal_t *current_goal;

	
	current_goal = FifoCurrentGoal();

	if(current_goal->type == TYPE_PWM && current_goal->data.pwm_data.autoStop)
			block_time = BLOCK_TIME_AUTO_STOP; 

	now = timeMillis();
	if (now - last_time < block_time)
		return;
	last_time = now;
	
	if (current_goal->type == NO_GOAL  || 
		current_goal->type == TYPE_SPD ||
	   (current_goal->type == TYPE_PWM && !current_goal->data.pwm_data.autoStop))
		goto end;

	if (fifo.current_goal != last_goal_nr) {
		last_goal_nr = fifo.current_goal;
		goto end;
	}
	

	// goals type is pos, pwm or angle, goal didn't change and 
	// last calculation was at least BLOCK_TIME ms ago

	dist = sqrt(pow(current_pos.x - last_pos.x, 2) + pow(current_pos.y - last_pos.y, 2));
	dist += abs(current_pos.angle - last_pos.angle)*ENTRAXE_ENC/2.0;
	if (dist < BLOCK_MIN_DIST) {
		// we did not move enough, we are probably blocked, 
		// consider the goal reached
		current_goal->is_reached = 1;
	}
	
end:
	last_pos = current_pos;
#endif
}
