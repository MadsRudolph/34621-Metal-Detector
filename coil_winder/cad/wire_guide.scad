// =============================================================================
// Wire Guide Carriage - Slides on rail, guides wire to coil
// =============================================================================

include <parameters.scad>

// Rail dimensions (simple rectangular rail)
rail_width = 8;
rail_height = 4;

module wire_guide_carriage() {
    guide_hole_dia = 3;  // Wire guide hole
    linkage_hole_dia = 3;  // For push rod connection

    difference() {
        union() {
            // Main body
            translate([-carriage_width/2, -carriage_depth/2, 0])
                cube([carriage_width, carriage_depth, carriage_height]);

            // Wire guide tube extending down
            translate([0, carriage_depth/2 - 5, -15])
                cylinder(d=8, h=15);

            // Linkage arm (connects to servo push rod)
            translate([-carriage_width/2 - 10, -5, carriage_height - 5])
                cube([15, 10, 5]);
        }

        // Rail slot (slides on rail)
        translate([-(rail_width + slide_fit*2)/2, -carriage_depth/2 - 0.1, -0.1])
            cube([rail_width + slide_fit*2, rail_height + slide_fit + 0.1, carriage_height + 0.2]);

        // Wire guide hole
        translate([0, carriage_depth/2 - 5, -15.1])
            cylinder(d=guide_hole_dia, h=carriage_height + 15.2);

        // Linkage pivot hole
        translate([-carriage_width/2 - 5, 0, carriage_height - 2.5])
            rotate([0, 0, 0])
                cylinder(d=linkage_hole_dia, h=10, center=true);
    }
}

// Guide rail (for reference)
module guide_rail() {
    color("gray")
    translate([-guide_rail_length/2, -carriage_depth/2, 0])
        cube([guide_rail_length, rail_height, rail_width]);
}

// Render for export
wire_guide_carriage();

// Show rail for reference (comment out for export)
// translate([0, 0, -rail_width]) guide_rail();
