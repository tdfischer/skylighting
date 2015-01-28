// Diameter of the mounting screw
SCREW_DIAMETER = 3; // [0:100]

// Width of your LED strip in millimeters
STRIP_WIDTH = 21; // [0:100]

// Thickness of yoru LED strip in millimeters
STRIP_THICKNESS = 4; // [1:100]

// How wide (along the strip) you want your clip to be in millimeters
CLIP_LENGTH = 13; // [1:100]

// How durable you'd like this to be in millimeters
THICKNESS=3; //[1:100]

// How many screw mounts to have
SCREW_MOUNT_COUNT = 2; //[0, 1, 2]

// Use support bridge?
USE_SUPPORT_BRIDGE = 0; //[0, 1]

difference() {
	translate([-THICKNESS, 0.1, 0.01]) cube(size=[STRIP_WIDTH+THICKNESS*2, CLIP_LENGTH+THICKNESS, STRIP_THICKNESS+THICKNESS]);
	difference() {
		translate([THICKNESS/2, 0, 0]) minkowski() {
			cube(size=[STRIP_WIDTH-THICKNESS, CLIP_LENGTH*2, STRIP_THICKNESS-THICKNESS/2]);
			sphere(r=THICKNESS/2, $fn=80);
		}
		if (USE_SUPPORT_BRIDGE > 0) translate([STRIP_WIDTH/2-THICKNESS/4, -CLIP_LENGTH/2, 0]) cube(size=[THICKNESS/2, CLIP_LENGTH*2, STRIP_THICKNESS+1]);
	}
	translate([0, CLIP_LENGTH/2-(CLIP_LENGTH*0.75-THICKNESS)/2, 0]) cube(size=[STRIP_WIDTH, CLIP_LENGTH*0.75, STRIP_THICKNESS*2]);
}

if (SCREW_MOUNT_COUNT > 0) {
	screwMount();
	if (SCREW_MOUNT_COUNT > 1) translate([STRIP_WIDTH, 0, 0]) mirror([1, 0, 0]) screwMount();
}


module screwMount() {
	translate([-THICKNESS*2.5, CLIP_LENGTH/2+THICKNESS/2, 0]) difference() {
		union() {
			cylinder(r=SCREW_DIAMETER+THICKNESS, h=STRIP_THICKNESS/2, $fn=80);
			difference() {
				translate([THICKNESS, -CLIP_LENGTH/2, 0]) cube(size=[THICKNESS, CLIP_LENGTH, STRIP_THICKNESS+THICKNESS]);
				translate([SCREW_DIAMETER/4, CLIP_LENGTH, THICKNESS*2]) rotate([90, 0, 0]) cylinder(r=STRIP_THICKNESS, h=CLIP_LENGTH*2, $fn=80);
			}
		}
		translate([0, 0, -0.1]) cylinder(r=SCREW_DIAMETER, h=STRIP_THICKNESS, $fn=80);
	}
}