// =============================================================================
// Encoder Wheel - Single slot for ITR8307 turn counting
// =============================================================================

include <parameters.scad>

module encoder_wheel() {
    difference() {
        // Main disc
        cylinder(d=encoder_wheel_dia, h=encoder_wheel_thickness);

        // Center hole for M8 spindle (tight fit with set screw)
        translate([0, 0, -0.1])
            cylinder(d=spindle_dia + clearance, h=encoder_wheel_thickness + 0.2);

        // Slot that blocks the IR beam
        translate([encoder_wheel_dia/2 - encoder_slot_depth/2, -encoder_slot_width/2, -0.1])
            cube([encoder_slot_depth + 1, encoder_slot_width, encoder_wheel_thickness + 0.2]);

        // Set screw hole (M3) - to lock onto spindle
        translate([0, spindle_dia/2 + 2, encoder_wheel_thickness/2])
            rotate([90, 0, 0])
                cylinder(d=m3_bolt_dia, h=10);

        // Nut trap for set screw
        translate([0, spindle_dia/2 + 5, encoder_wheel_thickness/2])
            rotate([90, 30, 0])
                cylinder(d=m3_nut_dia, h=m3_nut_height, $fn=6);
    }
}

// Render for export
encoder_wheel();
