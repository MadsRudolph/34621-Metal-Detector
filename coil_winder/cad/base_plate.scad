// =============================================================================
// Base Plate - Main frame for coil winder
// =============================================================================

include <parameters.scad>

// Positions for components
bearing_mount_offset_y = 20;
servo_mount_x = -base_length/2 + 40;
servo_mount_y = base_width/2 - 35;
sensor_mount_x = base_length/2 - 40;

module base_plate() {
    difference() {
        // Main plate
        translate([-base_length/2, -base_width/2, 0])
            cube([base_length, base_width, base_thickness]);

        // Bearing mount holes (left side)
        translate([-spindle_span/2, 0, 0]) {
            for (x = [-12.5, 12.5]) {
                translate([x, 0, -0.1])
                    cylinder(d=m3_bolt_dia, h=base_thickness + 0.2);
            }
        }

        // Bearing mount holes (right side)
        translate([spindle_span/2, 0, 0]) {
            for (x = [-12.5, 12.5]) {
                translate([x, 0, -0.1])
                    cylinder(d=m3_bolt_dia, h=base_thickness + 0.2);
            }
        }

        // Servo mount holes
        translate([servo_mount_x, servo_mount_y, 0]) {
            for (x = [-12, 12]) {
                for (y = [-12, 12]) {
                    translate([x, y, -0.1])
                        cylinder(d=m3_bolt_dia, h=base_thickness + 0.2);
                }
            }
        }

        // Sensor mount holes
        translate([sensor_mount_x, bearing_mount_offset_y, 0]) {
            for (x = [-7.5, 7.5]) {
                translate([x, 0, -0.1])
                    cylinder(d=m3_bolt_dia, h=base_thickness + 0.2);
            }
        }

        // Guide rail mount slot
        translate([servo_mount_x + 30, servo_mount_y, -0.1])
            cube([guide_rail_length + 10, 8, base_thickness + 0.2]);

        // Cable routing holes
        translate([0, -base_width/2 + 15, -0.1])
            cylinder(d=10, h=base_thickness + 0.2);
        translate([servo_mount_x, servo_mount_y - 20, -0.1])
            cylinder(d=8, h=base_thickness + 0.2);

        // Weight reduction / grip holes
        for (x = [-80, -40, 40, 80]) {
            for (y = [-30, 30]) {
                translate([x, y, -0.1])
                    cylinder(d=15, h=base_thickness + 0.2);
            }
        }

        // Corner mounting holes (for clamping to table)
        for (x = [-base_length/2 + 10, base_length/2 - 10]) {
            for (y = [-base_width/2 + 10, base_width/2 - 10]) {
                translate([x, y, -0.1])
                    cylinder(d=5, h=base_thickness + 0.2);
            }
        }
    }
}

// Render for export
base_plate();
