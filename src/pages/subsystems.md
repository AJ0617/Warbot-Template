# Subsystem Architecture

Every robot has different subsystems such as a drivetrain or a manipulator or a claw. 
Each subsystem plays a big part in a match, whether you win by a large amount or a small amount.  
It's also important to keep each subsystem organized not only mechanically but also in your code. 
It should be easy to dubug a bug that happens during a match, such as auto timing not working properly 
or your robot's arm goes too high.

This can be solved by creating subsystems in your code. 
Subsystems help take your code from unorganized and all over the place to organized and 
easy to find specific things for each subsystem. For example if your arm control does not work as intended during a match, 
you can open your code and open the arm subsystem file to debug the arm control instead of looking through hundreds of lines 
of code in a project without different subsystem files. That way you are ready for the next match and it BEAR-LY takes any time.

