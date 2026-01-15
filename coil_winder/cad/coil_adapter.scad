// =============================================================================
// Coil Adapter - Holds coil form on spindle
// Parametric for TX (200mm), RX (80mm), Buck (120mm)
// =============================================================================

include <parameters.scad>

// Select coil type: "TX", "RX", or "BUCK"
coil_type = "TX";

// Get dimensions based on coil type
coil_dia = (coil_type == "TX") ? tx_coil_dia :
           (coil_type == "RX") ? rx_coil_dia : buck_coil_dia;

coil_width = (coil_type == "TX") ? tx_coil_width :
             (coil_type == "RX") ? rx_coil_width : buck_coil_width;

// Adapter dimensions
adapter_hub_dia = 30;
adapter_hub_height = 20;
flange_thickness = 3;
spoke_count = 4;
spoke_width = 10;
spoke_thickness = 5;

module coil_adapter() {
    difference() {
        union() {
            // Center hub
            cylinder(d=adapter_hub_dia, h=adapter_hub_height);

            // Flange (wire stop)
            cylinder(d=coil_dia + 10, h=flange_thickness);

            // Spokes connecting hub to outer ring
            for (i = [0:spoke_count-1]) {
                rotate([0, 0, i * 360/spoke_count])
                    translate([0, -spoke_width/2, 0])
                        cube([coil_dia/2, spoke_width, spoke_thickness]);
            }

            // Outer ring (where wire is wound)
            difference() {
                cylinder(d=coil_dia + 6, h=coil_width + flange_thickness);
                translate([0, 0, flange_thickness])
                    cylinder(d=coil_dia - 6, h=coil_width + 1);
            }

            // Second flange at top
            translate([0, 0, coil_width + flange_thickness - flange_thickness])
                difference() {
                    cylinder(d=coil_dia + 10, h=flange_thickness);
                    translate([0, 0, -0.1])
                        cylinder(d=coil_dia - 10, h=flange_thickness + 0.2);
                }
        }

        // Spindle hole
        translate([0, 0, -0.1])
            cylinder(d=spindle_dia + clearance, h=adapter_hub_height + 0.2);

        // Set screw hole (M3) to lock on spindle
        translate([adapter_hub_dia/2 + 2, 0, adapter_hub_height/2])
            rotate([0, -90, 0])
                cylinder(d=m3_bolt_dia, h=15);

        // Nut trap
        translate([adapter_hub_dia/2 - 4, 0, adapter_hub_height/2])
            rotate([0, -90, 0])
                cylinder(d=m3_nut_dia, h=m3_nut_height, $fn=6);

        // Weight reduction holes in flange
        for (i = [0:7]) {
            rotate([0, 0, i * 45 + 22.5])
                translate([coil_dia/2 - 15, 0, -0.1])
                    cylinder(d=15, h=flange_thickness + 0.2);
        }
    }
}

// Render for export
coil_adapter();

// Echo the current coil type
echo(str("Coil adapter for: ", coil_type, " (", coil_dia, "mm diameter)"));
