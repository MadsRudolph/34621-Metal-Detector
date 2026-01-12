// =============================================================================
// Print Plate - All parts arranged for 3D printing
// DTU 34621 Metal Detector Project
//
// Print Settings:
// - Layer height: 0.2mm
// - Infill: 20-30%
// - Walls: 3 perimeters
// - No supports needed for most parts
// =============================================================================

include <parameters.scad>

use <bearing_mount.scad>
use <encoder_wheel.scad>
use <hand_crank.scad>
use <servo_mount.scad>
use <wire_guide.scad>
use <push_rod.scad>
use <sensor_mount.scad>

// Note: Base plate and coil adapters should be printed separately due to size

// -----------------------------------------------------------------------------
// Small Parts Plate (fits 200x200mm bed)
// -----------------------------------------------------------------------------

module small_parts_plate() {
    // Bearing mount x2
    translate([0, 0, 0])
        bearing_mount();
    translate([50, 0, 0])
        bearing_mount();

    // Encoder wheel
    translate([100, 20, 0])
        encoder_wheel();

    // Sensor mount
    translate([150, 20, 0])
        sensor_mount();

    // Push rod
    translate([0, 60, 0])
        push_rod();

    // Wire guide carriage
    translate([80, 70, 15])
        rotate([180, 0, 0])  // Print upside down for better bridging
            wire_guide_carriage();
}

// -----------------------------------------------------------------------------
// Medium Parts Plate
// -----------------------------------------------------------------------------

module medium_parts_plate() {
    // Hand crank (print on side for strength)
    translate([0, 0, 0])
        hand_crank();

    // Servo mount
    translate([80, 0, 0])
        servo_mount();
}

// -----------------------------------------------------------------------------
// Select what to show/export
// -----------------------------------------------------------------------------

// Uncomment the plate you want to export:

small_parts_plate();

// translate([0, 120, 0]) medium_parts_plate();

// For large parts, open individual files:
// - base_plate.scad (print separately, may need to split)
// - coil_adapter.scad (change coil_type and export each)

echo("=== Print Plate ===");
echo("Small parts plate shown");
echo("Estimated print time: ~4-6 hours");
echo("Estimated filament: ~80g");
