#include "main.h"
#include "warbotTemplate/util.hpp"

warbots::Drive drive(
	{10},// Left Motors ID
	{-4}, //Right Motors ID
	21, //IMU/Inertial Sensor Port
	1.0, //Wheel Diameter
	1.0 //Motor Ticks
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
	
    pros::lcd::initialize();
	pros::delay(2000);
	pros::lcd::clear_line(7);
    logo_task.remove();
	register_autons();
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
		warbots::drawLogo();
		drive.control(master);
		pros::delay(20);
	}
}
