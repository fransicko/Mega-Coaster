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
	