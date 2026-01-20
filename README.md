This project is a small autonomous robot car that avoids falling off edges.
Two ultrasonic sensors (left and right) scan the ground in front of the car.
During motion, the Arduino continuously compares distance readings.
If the left sensor sees the floor disappear, the car stops, reverses a bit, and turns to the right.
If the right sensor loses the floor, it stops and turns left. If both sensors detect no floor, it means the edge is straight ahead, so the car reverses and re-orients.
Motor speed and direction are handled by an L298N motor driver. I had many issues along the way: sensors reacting slowly, motors jerking due to friction, a power cable snapping off the driver in early testing, and debugging logic that only worked when the wheels were in the air.
After tuning the distance thresholds, fixing wiring, re-soldering connections, and adding timing adjustments, the system finally navigates edges reliably without falling off.
The wiring schematic can be found in the github page. I made add some more time because i did maintanence on the car and fixed some errors
