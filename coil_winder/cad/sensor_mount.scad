// =============================================================================
// ITR8307 Sensor Mount - Positions optical sensor for encoder wheel
// =============================================================================

include <parameters.scad>

mount_width = 20;
mount_depth = 15;
mount_height = 30;  // Height to align with encoder wheel

module sensor_mount() {
    difference() {
        union() {
            // Base
            cube([mount_width, mount_depth, wall_thickness]);

            // Vertical arm
            cube([wall_thickness * 2, mount_depth, mount_height]);

            // Sensor holder platform
            translate([0, 0, mount_height - wall_thickness])
                cube([mount_width, mount_depth, wall_thickness]);
        }

        // ITR8307 pocket
        translate([wall_thickness * 2, (mount_depth - itr8307_depth)/2, mount_height - wall_thickness - 0.1]) {
            // Sensor body
            cube([itr8307_width + clearance*2, itr8307_depth + clearance*2, wall_thickness + 0.2]);
        }

        // Slot for encoder wheel to pass through
        translate([wall_thickness * 2 + itr8307_width/2 - itr8307_slot_width/2 - clearance,
                   -0.1,
                   mount_height - wall_thickness - encoder_wheel_thickness - 5])
            cube([itr8307_slot_width + clearance*2, mount_depth + 0.2, encoder_wheel_thickness + 10]);

        // Wire routing hole
        translate([mount_width/2, mount_depth/2, -0.1])
            cylinder(d=5, h=wall_thickness + 0.2);

        // Mounting holes (M3)
        for (x = [5, mount_width - 5]) {
            translate([x, mount_depth/2, -0.1])
                cylinder(d=m3_bolt_dia, h=wall_thickness + 0.2);
        }
    }
}

// ITR8307 visualization (for reference)
module itr8307_visual() {
    color("darkgray")
    translate([wall_thickness * 2, (mount_depth - itr8307_depth)/2, mount_height - wall_thickness])
        cube([itr8307_width, itr8307_depth, itr8307_height]);
}

// Render for export
sensor_mount();

// Show sensor (comment out for export)
// itr8307_visual();
