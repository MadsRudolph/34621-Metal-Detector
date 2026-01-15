// =============================================================================
// Servo Mount - Holds SG90/MG90S servo for wire guide traverse
// =============================================================================

include <parameters.scad>

mount_base_width = 40;
mount_base_depth = 30;
mount_base_height = 5;

module servo_mount() {
    difference() {
        union() {
            // Base plate
            cube([mount_base_width, mount_base_depth, mount_base_height]);

            // Servo cradle walls
            translate([0, 0, mount_base_height]) {
                // Left wall
                cube([wall_thickness, mount_base_depth, servo_body_h + 5]);
                // Right wall
                translate([mount_base_width - wall_thickness, 0, 0])
                    cube([wall_thickness, mount_base_depth, servo_body_h + 5]);
            }

            // Front support
            translate([0, 0, mount_base_height])
                cube([mount_base_width, wall_thickness, servo_tab_offset]);
        }

        // Servo body cutout
        translate([wall_thickness + clearance, wall_thickness, mount_base_height])
            cube([servo_body_w + clearance*2, servo_body_d + clearance*2 + 10, servo_body_h + 10]);

        // Tab slots
        translate([(mount_base_width - servo_tab_w)/2 - clearance, -0.1, mount_base_height + servo_tab_offset])
            cube([servo_tab_w + clearance*2, wall_thickness + 0.2, servo_tab_h + clearance*2]);

        // Mounting holes (M3)
        for (x = [8, mount_base_width - 8]) {
            for (y = [8, mount_base_depth - 8]) {
                translate([x, y, -0.1])
                    cylinder(d=m3_bolt_dia, h=mount_base_height + 0.2);
            }
        }

        // Wire routing slot
        translate([mount_base_width/2 - 3, -0.1, -0.1])
            cube([6, mount_base_depth + 0.2, mount_base_height + 0.2]);
    }
}

// Render for export
servo_mount();
