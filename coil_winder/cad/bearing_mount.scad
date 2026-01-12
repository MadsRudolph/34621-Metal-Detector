// =============================================================================
// Bearing Mount - Holds 608ZZ bearing for spindle
// =============================================================================

include <parameters.scad>

module bearing_mount() {
    difference() {
        union() {
            // Main body
            hull() {
                // Base
                translate([0, 0, wall_thickness/2])
                    cube([bearing_mount_width, bearing_mount_depth, wall_thickness], center=true);

                // Top with bearing pocket
                translate([0, 0, bearing_mount_height - bearing_od/2 - wall_thickness])
                    rotate([90, 0, 0])
                        cylinder(d=bearing_od + wall_thickness*2, h=bearing_mount_depth, center=true);
            }
        }

        // Bearing pocket (press fit)
        translate([0, 0, bearing_mount_height - bearing_od/2 - wall_thickness])
            rotate([90, 0, 0])
                cylinder(d=bearing_od + press_fit*2, h=bearing_width + clearance, center=true);

        // Spindle through-hole
        translate([0, 0, bearing_mount_height - bearing_od/2 - wall_thickness])
            rotate([90, 0, 0])
                cylinder(d=spindle_dia + clearance*2, h=bearing_mount_depth + 1, center=true);

        // Mounting holes (M3)
        for (x = [-bearing_mount_width/2 + 5, bearing_mount_width/2 - 5]) {
            translate([x, 0, -0.1])
                cylinder(d=m3_bolt_dia, h=wall_thickness + 0.2);
        }
    }
}

// Render for export
bearing_mount();
