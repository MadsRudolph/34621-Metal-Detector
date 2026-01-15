// =============================================================================
// Coil Winder Assembly - Complete visualization
// DTU 34621 Metal Detector Project
// =============================================================================

include <parameters.scad>

// Import individual parts as modules
use <bearing_mount.scad>
use <encoder_wheel.scad>
use <hand_crank.scad>
use <servo_mount.scad>
use <wire_guide.scad>
use <push_rod.scad>
use <sensor_mount.scad>
use <base_plate.scad>

// Coil adapter inline (since it's parametric)
adapter_hub_dia = 30;
adapter_hub_height = 20;
flange_thickness = 3;

module coil_adapter_simple(dia, width) {
    color("orange", 0.7)
    difference() {
        union() {
            cylinder(d=adapter_hub_dia, h=adapter_hub_height);
            cylinder(d=dia + 10, h=flange_thickness);
            difference() {
                cylinder(d=dia + 6, h=width + flange_thickness);
                translate([0, 0, flange_thickness])
                    cylinder(d=dia - 6, h=width + 1);
            }
        }
        translate([0, 0, -0.1])
            cylinder(d=spindle_dia + clearance, h=adapter_hub_height + 0.2);
    }
}

// -----------------------------------------------------------------------------
// Assembly Positions
// -----------------------------------------------------------------------------

servo_mount_x = -base_length/2 + 40;
servo_mount_y = base_width/2 - 35;
sensor_mount_x = base_length/2 - 40;
sensor_mount_y = 20;

spindle_height = base_thickness + bearing_mount_height - bearing_od/2 - wall_thickness;

// -----------------------------------------------------------------------------
// Visualization Helpers
// -----------------------------------------------------------------------------

module spindle_visual() {
    color("silver")
    rotate([0, 90, 0])
        cylinder(d=spindle_dia, h=spindle_length, center=true);
}

module bearing_visual() {
    color("gray")
    rotate([90, 0, 0])
        difference() {
            cylinder(d=bearing_od, h=bearing_width, center=true);
            cylinder(d=bearing_id, h=bearing_width + 1, center=true);
        }
}

module servo_visual() {
    color("blue", 0.8) {
        // Body
        translate([-servo_body_w/2, -servo_body_d/2, 0])
            cube([servo_body_w, servo_body_d, servo_body_h]);
        // Tab
        translate([-servo_tab_w/2, -servo_body_d/2, servo_tab_offset])
            cube([servo_tab_w, 2, servo_tab_h]);
        // Output shaft
        translate([servo_shaft_offset, 0, servo_body_h])
            cylinder(d=5, h=5);
        // Horn
        translate([servo_shaft_offset, 0, servo_body_h + 3])
            hull() {
                cylinder(d=7, h=2);
                translate([servo_horn_length, 0, 0])
                    cylinder(d=4, h=2);
            }
    }
}

// -----------------------------------------------------------------------------
// Complete Assembly
// -----------------------------------------------------------------------------

module complete_assembly() {
    // Base plate
    color("lightblue", 0.9)
        base_plate();

    // Left bearing mount
    translate([-spindle_span/2, 0, base_thickness])
        color("green", 0.8)
            bearing_mount();

    // Right bearing mount
    translate([spindle_span/2, 0, base_thickness])
        color("green", 0.8)
            bearing_mount();

    // Spindle
    translate([0, 0, spindle_height])
        spindle_visual();

    // Bearings
    translate([-spindle_span/2, 0, spindle_height])
        bearing_visual();
    translate([spindle_span/2, 0, spindle_height])
        bearing_visual();

    // Encoder wheel
    translate([spindle_span/2 - 20, 0, spindle_height])
        rotate([0, 90, 0])
            color("red", 0.8)
                encoder_wheel();

    // Sensor mount
    translate([sensor_mount_x, sensor_mount_y, base_thickness])
        color("purple", 0.8)
            sensor_mount();

    // Hand crank (on left side)
    translate([-spindle_span/2 - 30, 0, spindle_height])
        rotate([0, -90, 0])
            color("yellow", 0.8)
                hand_crank();

    // TX Coil adapter (on spindle)
    translate([0, 0, spindle_height])
        rotate([0, 90, 0])
            coil_adapter_simple(tx_coil_dia, tx_coil_width);

    // Servo mount
    translate([servo_mount_x, servo_mount_y, base_thickness])
        color("cyan", 0.8)
            servo_mount();

    // Servo (visual)
    translate([servo_mount_x + 12, servo_mount_y + 6, base_thickness + 5])
        servo_visual();

    // Wire guide carriage
    translate([servo_mount_x + 60, servo_mount_y, base_thickness + 15])
        color("magenta", 0.8)
            wire_guide_carriage();

    // Push rod
    translate([servo_mount_x + 12 + servo_shaft_offset + servo_horn_length,
               servo_mount_y,
               base_thickness + 5 + servo_body_h + 4])
        color("brown", 0.8)
            push_rod();
}

// Render assembly
complete_assembly();

// Print summary
echo("=== Coil Winder Assembly ===");
echo(str("Base size: ", base_length, " x ", base_width, " x ", base_thickness, " mm"));
echo(str("Spindle span: ", spindle_span, " mm"));
echo(str("Spindle height: ", spindle_height, " mm"));
