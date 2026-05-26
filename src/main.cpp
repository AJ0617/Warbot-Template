#include "main.h"
#include "warbotTemplate/util.hpp"

warbots::Drive drive(
	{10},  // Left Motors ID
	{-4},  // Right Motors ID
	3.25,  // Wheel Diameter
	1.0,   // Gear Ratio
	false,  // Are you using an IMU on the Robot?
	0   // If you are using an IMU, put the motor port here, if you are not using an IMU, leave at 0
);



/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::Task logo_task([]() {
		while (true) {
			drawLogo();
			pros::delay(20);
			warbots::screenPrint("Warbot-Template", 7, pros::E_TEXT_LARGE_CENTER);
			
		}
	});

	//Have Rotation Sensors/Odom Pods on your drivetrain?
	//Add them Here!!
	// drive.addHorizontalTrackingWheel(15, 3.25);
	// drive.addVerticalTrackingWheel(16, 3.25);

	drive.setOdomConfig(warbots::Drive::odomConfig::MOTOR_ENCODERS);
	drive.initImu();
	drive.resetPose();
	
	register_autons();
    pros::lcd::initialize();
	pros::delay(2000);
    logo_task.remove();
	selector.init();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	selector.selected_auton_call();
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	
	drive.setDriveType(warbots::Drive::TANK);
	// drive.setDriveType(warbots::Drive::SINGLE_ARCADE);
	// drive.setDriveType(warbots::Drive::FLIPPED_SINGLE_ARCADE);
	// drive.setDriveType(warbots::Drive::SPLIT_ARCADE);
	// drive.setDriveType(warbots::Drive::FLIPPED_SPLIT_ARCADE);
	while (true) {
		warbots::screenPrint("Arm: " + warbots::doubleToString(arm.get_position(), 2), 2, pros::E_TEXT_MEDIUM);
		warbots::drawLogo();
		drive.control(master);
		examplePIDFunction(1400); // drive arm to 200 encoder ticks
		pros::delay(20);
	}
}
