// =============================================================================
// Push Rod - Connects servo horn to wire guide carriage
// =============================================================================

include <parameters.scad>

rod_length = 50;
rod_width = 8;
rod_thickness = 3;
hole_dia = 2.5;  // For M2 or wire pivot

module push_rod() {
    difference() {
        // Rod body
        hull() {
            cylinder(d=rod_width, h=rod_thickness);
            translate([rod_length, 0, 0])
                cylinder(d=rod_width, h=rod_thickness);
        }

        // Pivot holes
        translate([0, 0, -0.1])
            cylinder(d=hole_dia, h=rod_thickness + 0.2);
        translate([rod_length, 0, -0.1])
            cylinder(d=hole_dia, h=rod_thickness + 0.2);

        // Length adjustment slot (optional)
        translate([rod_length - 15, -1.5, -0.1])
            cube([10, 3, rod_thickness + 0.2]);
    }
}

// Render for export
push_rod();
