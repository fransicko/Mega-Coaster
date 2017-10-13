NAME

	Guild Name: Twice's Meme Oasis
	Guild Members: Michael Villafuerte, Michael Niemeyer, Khanh Duong

	Midterm Project - Roller Coaster Tycoon

DESCRIPTION
	
		This code will display a cloased C^0 and C^1 curves that will allow two of our heros to travers in a scenic enviroment.
	One hero, Micael Villafuerte's, will go along the track following a arc-length parameterization of the Bezier curve,
	while the other hero, Khanh Duong's, will follow the standard Bezier curve. 
		The user has the ability to move around the 
	terrain comrpised of Bezier Surfaces in Michael Niemeyer's hero. Along with that the user has the ability to switch between 
	different camera angles. These angles are of the different hero's that are moving on the track, on the ground,and one free 
	camera to allow the user to wander around. Should the user want to they can see what the hero see's in first person view.

CONTROLS

	General:
		c : Turns on control cage and Bezier curves for all 3 mascot
	
	Camera:
		Left Click : Rotate Camera
		Control + Left Click : Zoom Camera, not available in Free Camera Mode
		
		1 : Arcball on Michael Niemeyer
		2 : Arcball on Khanh Duong
		3 : Arcball on Michael Villafuerte
		4 : Free Camera
		
		f : Activate first person viewport (starts on Michael Niemeyer)
		j : Turn FPV on Michael Niemeyer
		k : Turn FPV on Khanh Duong
		l : Turn FPV on Michael Villafuerte
		
	Movement:
		W/D : Move forward and backward with Michael Niemeyer's car.
		A/S : Turn left and right with Michael Niemeyer's car.
		Up/Down : Move Camera, only available in Free Camera mode.
		
WORLD FILE

	Format:
		<number of control points for Bezier Patch, recommend 16>
		x1,y1,z1
		...
		xn,yn,zn
		<number of control points for Bezier Curve for coaster>
		x1,y1,z1
		...
		xn,yn,zn
		<number of objects>
		<object1 type>,x,y,z,<object1 size>
		...
		<objectn type>,x,y,z,<objectn size>
		
	Important: No white spaces
	For object type, pass in an integer:
		1 = Boulder
		2 = Pyramid
	x,y,z denotes location of object
	
DISTRIBUTION OF WORK

	Michael Niemeyer : Michael worked on creating the Bezier Surfaces to allow his hero to move along the curves of the enviroment,
						in addition to that Michael worked on and created a WorldFile2 that allow a new world scene.
						
	Michael Villafuerte : Michael worked on creating the arc length parameterization path that his hero will follow along the track.
	
	Khanh Duong : Khanh worked on creating the track that both his and Michael V's hero traverse. In addition he is responsible for creating the 
					different camera views that the user can switch between and the first person camera view the user can toggle on and off. He
					also created a the WorldFile1 that the user can set for the scenery.
					
INSTRUCTION
	
	Update the Makefile to fit the environment as needed. Make the "rct.exe". To execute, a WorldFile has to be past in, instruction on creating
	one is listed up. Execute with command > rct.exe WorldFile.csv
	
MISC

	The assignment took roughly 20 hours, split among all members.
	Both the lab and homework help tremendously in doing the project. Doing the extra credits in particular for the assignments really paid off.
	10 points for both categories.
	"Fun" aspect of this assignment is difficult to gauge. On one hand, it was interactive and interesting, but it came at a time that was hectic.
	The assignment is a time sink. To fully enjoy the project and design a perfect park, alot of time have to be invested, which we could not manage.
	Eventually, it decayed to just drawing something that works. We'd rate it about 7, because it was still somewhat enjoyable.