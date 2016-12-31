
use <../../XasCode/OpenSCAD/TagSystem/Tagging.scad>


$fs = 1;

fwDims = [78, 54, 6];

wallSize = 1;

arduinoScrews = [[0, 0], [1.3, 48.2], [52.1, 33], [52.1, 5.1]];
arduinoDims = [52.1, 48.2, 8.2];

module cSquare(dims, delta) {
	offset(delta = delta, chamfer = false) square([dims[0], dims[1]]);
}

module eMount() {
	tag("positive") linear_extrude(height = fwDims[2] + wallSize) 
		cSquare(fwDims, wallSize);
	
	tag("negative") translate([0, 0, fwDims[2]]) 
		linear_extrude(height = 100)
		cSquare(fwDims, 0);
	
	tag("negative") translate([0, 0, 0.5]) 
		linear_extrude(height = 100) 
		cSquare(fwDims, -wallSize);
}

module arduScrewMounts() {
	for(x = arduinoScrews) translate(x) {
		tag("negative") circle(d = 3.2);
		tag("positive") circle(d = 3.2 + 2*wallSize);
	}
}

module arduinoMount() {
	linear_extrude(height = arduinoDims[2]) arduScrewMounts();
	
	tag("positive") {
		linear_extrude(height = 0.5) 
			offset(r = 1.6 + wallSize) square([arduinoDims[0], arduinoDims[1]]);
		
		linear_extrude(height = arduinoDims[2]/2) difference() {
			offset(r = 1.6 + wallSize) square([arduinoDims[0], arduinoDims[1]]);
			
			offset(r = 1.6) square([arduinoDims[0], arduinoDims[1]]);
		}
	}
}

module comMounts() {
	translate([1.6 + wallSize + 14, -arduinoDims[1] - 1.6 - wallSize - 2.5, 0]) arduinoMount();
	eMount();
}

module base() {
	tag("positive") linear_extrude(height = 0.5) 
	hull() projection(false) showOnly("positive") comMounts();
}

taggedDifference("positive", "negative", "neutral") {
	comMounts();
	base();
}