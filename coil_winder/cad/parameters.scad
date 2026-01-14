// =============================================================================
// Coil Winder - Common Parameters
// DTU 34621 Metal Detector Project
// =============================================================================

// -----------------------------------------------------------------------------
// Hardware Dimensions
// -----------------------------------------------------------------------------

// M8 Threaded Rod (Spindle)
spindle_dia = 8;
spindle_length = 300;

// 608ZZ Bearing (Skateboard bearing)
bearing_od = 22;
bearing_id = 8;
bearing_width = 7;

// M3 Hardware
m3_bolt_dia = 3.2;
m3_nut_dia = 6.4;
m3_nut_height = 2.4;
m3_head_dia = 5.5;
m3_head_height = 3;

// M8 Hardware
m8_nut_dia = 14.6;
m8_nut_height = 6.5;

// -----------------------------------------------------------------------------
// Servo Dimensions (SG90 / MG90S style)
// -----------------------------------------------------------------------------

servo_body_w = 23;
servo_body_h = 22.5;
servo_body_d = 12.5;
servo_tab_w = 32.5;
servo_tab_h = 2.5;
servo_tab_offset = 15.5;  // from bottom
servo_mount_hole_spacing = 28;
servo_shaft_offset = 5.5;  // from center of body
servo_horn_length = 15;

// -----------------------------------------------------------------------------
// ITR8307 Optical Sensor
// -----------------------------------------------------------------------------

itr8307_width = 10.8;
itr8307_depth = 5.2;
itr8307_height = 6.3;
itr8307_slot_width = 3;
itr8307_slot_depth = 4;

// -----------------------------------------------------------------------------
// Coil Dimensions
// -----------------------------------------------------------------------------

// TX Coil - 200mm diameter
tx_coil_dia = 200;
tx_coil_width = 18;
tx_coil_turns = 68;

// RX Coil - 80mm diameter
rx_coil_dia = 80;
rx_coil_width = 14;
rx_coil_turns = 170;

// Bucking Coil - 120mm diameter
buck_coil_dia = 120;
buck_coil_width = 18;
buck_coil_turns = 35;

// -----------------------------------------------------------------------------
// Frame Dimensions
// -----------------------------------------------------------------------------

base_length = 250;
base_width = 120;
base_thickness = 5;

bearing_mount_height = 40;
bearing_mount_width = 35;
bearing_mount_depth = 25;

// Distance between bearing centers
spindle_span = 200;

// -----------------------------------------------------------------------------
// Wire Guide / Traverse
// -----------------------------------------------------------------------------

guide_rail_length = 80;
guide_rail_width = 10;
guide_rail_height = 5;

carriage_width = 25;
carriage_depth = 20;
carriage_height = 15;

// -----------------------------------------------------------------------------
// Encoder Wheel
// -----------------------------------------------------------------------------

encoder_wheel_dia = 40;
encoder_wheel_thickness = 5;
encoder_slot_width = 5;
encoder_slot_depth = 12;

// -----------------------------------------------------------------------------
// General Tolerances
// -----------------------------------------------------------------------------

clearance = 0.3;        // General clearance for fitting parts
press_fit = 0.1;        // Tight fit
slide_fit = 0.4;        // Sliding fit

// -----------------------------------------------------------------------------
// Print Settings
// -----------------------------------------------------------------------------

wall_thickness = 2.4;   // 3 perimeters at 0.4mm nozzle
layer_height = 0.2;

$fn = 60;  // Circle resolution
