// =============================================================================
// Hand Crank - Attaches to M8 spindle
// =============================================================================

include <parameters.scad>

crank_arm_length = 60;
crank_arm_width = 15;
crank_arm_thickness = 8;
handle_dia = 12;
handle_length = 30;
hub_dia = 25;
hub_height = 15;

module hand_crank() {
    difference() {
        union() {
            // Hub
            cylinder(d=hub_dia, h=hub_height);

            // Crank arm
            translate([0, 0, hub_height - crank_arm_thickness])
                hull() {
                    cylinder(d=hub_dia, h=crank_arm_thickness);
                    translate([crank_arm_length, 0, 0])
                        cylinder(d=crank_arm_width, h=crank_arm_thickness);
                }

            // Handle
            translate([crank_arm_length, 0, hub_height])
                cylinder(d=handle_dia, h=handle_length);

            // Handle grip (slightly larger, rounded)
            translate([crank_arm_length, 0, hub_height + handle_length - 5])
                cylinder(d1=handle_dia, d2=handle_dia + 4, h=5);
            translate([crank_arm_length, 0, hub_height + handle_length])
                sphere(d=handle_dia + 4);
        }

        // Spindle hole
        translate([0, 0, -0.1])
            cylinder(d=spindle_dia + clearance, h=hub_height + 0.2);

        // Set screw hole (M3)
        translate([hub_dia/2 + 2, 0, hub_height/2])
            rotate([0, -90, 0])
                cylinder(d=m3_bolt_dia, h=15);

        // Nut trap
        translate([hub_dia/2 - 3, 0, hub_height/2])
            rotate([0, -90, 0])
                cylinder(d=m3_nut_dia, h=m3_nut_height, $fn=6);
    }
}

// Render for export
hand_crank();
