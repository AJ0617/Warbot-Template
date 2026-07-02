# Auto Drive

## Basics
The    `PID_driveInches()` function is used to move your robot forward and backwards


## Basic Drive Functions
To use the drive function, you need to include at least the distance you want to drive in inches

```
drive.PID_driveInches(12)
```


There are also other factors you can include such as:
- Max Speed

This is to limit your drivetrain so the max speed it can go is the number you set


`drive.PID_driveInches(12,100)`

- Drive Tolerance

This stops the drivetrain when it reaches a certian tolerance inches,
defaults to 0.5 inches

`drive.PID_driveInches(12,100, 0.5)`



- You can also add something called **Hold Heading**.

Hold Heading is to hold the angle of the robot just in case one motor is overpowering the other or something else happens.
All you have to do is set holdHeading to true and set the kP for it.
```
// Second to last Parameter is to turn on hold Heading, defaults to false
// Last Parameter is the kP for Hold Heading
drive.PID_Drive(12,127,0.5,true,0.5);
```




## PID
In Auto, it takes lots of tuning to make a really good auto, that's why we made it easy to tune and set PID values

All you have to do is these two lines of code:



Boom, Now you can drive

