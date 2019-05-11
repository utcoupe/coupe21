//delay for presure propagation
#define PNEU_DELAY 150 // in ms TODO:OPTIMIZE THIS
#define PUCK_TIME_TO_MOVE 600 // in ms TODO:OPTIMIZE THIS
#define PUCKS_DOOR_TIME_TO_MOVE 1000// in ms TODO:OPTIMIZE THIS
// ELECTROVANNES Ventouse pins
#define PUMP 1
#define TEST1 3
#define VENT1_TO_PUMP 2
#define VENT2_TO_PUMP 3
#define VENT3_TO_PUMP 4
#define VENT1_TO_AIR 5
#define VENT2_TO_AIR 6
#define VENT3_TO_AIR 7

//Scale door
#define SCALE_DOOR_CLOSE 0
#define SCALE_DOOR_OPEN 1

//Atom tower
#define TOWER_DOWN 0
#define TOWER_UP 1

//Back door
#define BACK_DOOR_CLOSE 0
#define BACK_DOOR_OPEN 1

//messages to ROS
#define AWAITING_ORDER_MSG 1
#define PUCKS_SUCKED_UP_MSG 2
#define PUCKS_SORTED_MSG 3
#define PUCKS_DOOR_RESET_MSG 4
#define TOWER_OPEN 5

//SELECTOR_PWM for pucks sorting
#define SELECTOR_PWM 8
#define SELECTOR_SCALE_POS 128 //TODO: set this
#define SELECTOR_PILE_POS 128 //TODO: set this
#define SELECTOR_TIME_TO_MOVE 1000 // in ms TODO:OPTIMIZE THIS
//atom tower for PR's pickup
#define TOWER_ELEVATOR_DELAY 5000 //TODO
#define TOWER_ELEVATOR_STEP_PIN 9
#define TOWER_ELEVATOR_DIR_PIN 10
#define TOWER_ELEVATOR_EN_PIN 11
#define TOWER_ELEVATOR_MIN_DELAY 1000 //TODO
#define TOWER_ELEVATOR_LIMIT_SWITCH A3


//atom tower for PR's pickup
#define PUCKS_DOOR_STEP_PIN 12
#define PUCKS_DOOR_DIR_PIN 13
#define PUCKS_DOOR_EN_PIN 14 
#define PUCKS_DOOR_MIN_DELAY 1000
#define PUCKS_DOOR_ISUP_PIN A1 
#define PUCKS_DOOR_ISDOWN_PIN A2