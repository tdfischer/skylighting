INCHES = 25.4;
STRIP_WIDTH = 0.65*INCHES;
STRIP_THICKNESS = 0.16*INCHES;
CLIP_LENGTH = 0.5*INCHES;
THICKNESS=3;
difference() {
	translate([-THICKNESS, 0.1, 0.01]) cube(size=[STRIP_WIDTH+THICKNESS*2, CLIP_LENGTH+THICKNESS, STRIP_THICKNESS+THICKNESS]);
	cube(size=[STRIP_WIDTH, CLIP_LENGTH*2, STRIP_THICKNESS]);
}


translate([-THICKNESS*2.5, CLIP_LENGTH/2+THICKNESS/2, 0]) difference() {
	union() {
		cylinder(r=0.1*INCHES+THICKNESS, h=STRIP_THICKNESS/2, $fn=80);
		translate([THICKNESS, -(0.1*INCHES+THICKNESS)*1.15, 0]) cube(size=[THICKNESS, CLIP_LENGTH, STRIP_THICKNESS*0.75]);
	}
	translate([0, 0, -0.1]) cylinder(r=0.1*INCHES, h=STRIP_THICKNESS, $fn=80);
}