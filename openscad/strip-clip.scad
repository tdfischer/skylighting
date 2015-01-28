INCHES = 25.4;

// Width of your LED strip in millimeters
STRIP_WIDTH = 21; // [0:100]

// Thickness of yoru LED strip in millimeters
STRIP_THICKNESS = 4; // [1:100]

// How wide (along the strip) you want your clip to be in millimeters
CLIP_LENGTH = 13; // [1:100]

// How durable you'd like this to be in millimeters
THICKNESS=3; //[1:100]

difference() {
	translate([-THICKNESS, 0.1, 0.01]) cube(size=[STRIP_WIDTH+THICKNESS*2, CLIP_LENGTH+THICKNESS, STRIP_THICKNESS+THICKNESS]);
	cube(size=[STRIP_WIDTH, CLIP_LENGTH*2, STRIP_THICKNESS]);
	translate([0, CLIP_LENGTH/2-(CLIP_LENGTH*0.75-THICKNESS)/2, 0]) cube(size=[STRIP_WIDTH, CLIP_LENGTH*0.75, STRIP_THICKNESS*2]);
}


translate([-THICKNESS*2.5, CLIP_LENGTH/2+THICKNESS/2, 0]) difference() {
	union() {
		cylinder(r=0.1*INCHES+THICKNESS, h=STRIP_THICKNESS/2, $fn=80);
		translate([THICKNESS, -(0.1*INCHES+THICKNESS)*1.15, 0]) cube(size=[THICKNESS, CLIP_LENGTH, STRIP_THICKNESS*0.75]);
	}
	translate([0, 0, -0.1]) cylinder(r=0.1*INCHES, h=STRIP_THICKNESS, $fn=80);
}