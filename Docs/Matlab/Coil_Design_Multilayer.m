%% Multi-Layer Solenoid Coil Design Calculator for VLF Metal Detector
% DTU Course 34621 - Electromagnetic Sensors and Digital Signal Processing
%
% For coils wound in MULTIPLE LAYERS around a cylinder
%
% Author: Metal Detector Project Team
% Date: January 2025

clear; clc; close all;

%% ========================================================================
%  MULTI-LAYER SOLENOID FORMULA
%  ========================================================================
%
%  Wheeler's formula for multi-layer solenoid:
%
%       L (µH) = (0.8 × r² × N²) / (6r + 9l + 10w)   [r, l, w in cm]
%
%  Where:
%       r = mean radius of the coil [cm]
%       l = axial length (height of winding) [cm]
%       w = radial build (depth/thickness of winding) [cm]
%       N = total number of turns
%
%  ========================================================================

%% ========================================================================
%  USER-ADJUSTABLE PARAMETERS
%  ========================================================================

% Operating parameters
f_tx = 2000;                    % TX frequency [Hz]
V_supply = 7.5;                 % Design voltage [V]

% =========================================================================
% TX COIL - READY TO IMPLEMENT
% =========================================================================
TX.wire_diameter = 0.52e-3;     % Wire diameter [m] (0.52 mm = AWG 24)
TX.wire_rho = 1.68e-8;          % Copper resistivity [Ohm*m]
TX.diameter = 250e-3;           % Form diameter [m] (250 mm)
TX.L_target = 15e-3;            % Target inductance [H] (15 mH)
TX.n_layers = 3;                % Number of layers

% =========================================================================
% RX COIL - PLACEHOLDER (update when dimensions known)
% =========================================================================
RX.wire_diameter = 0.32e-3;     % Wire diameter [m] (0.32 mm = AWG 28) - TBD
RX.wire_rho = 1.68e-8;          % Copper resistivity [Ohm*m]
RX.diameter = 80e-3;            % Form diameter [m] - TBD
RX.L_target = 10e-3;            % Target inductance [H] (10 mH) - requirement 16
RX.n_layers = 4;                % Number of layers - TBD

% =========================================================================
% BUCKING COIL - PLACEHOLDER (update when dimensions known)
% =========================================================================
Bucking.wire_diameter = 0.52e-3;  % Wire diameter [m] - TBD
Bucking.wire_rho = 1.68e-8;       % Copper resistivity [Ohm*m]
Bucking.diameter = 120e-3;        % Form diameter [m] - TBD (between TX and RX)
Bucking.n_layers = 2;             % Number of layers - TBD
Bucking.N_start = 40;             % Starting turns - TBD (adjust empirically)

%% ========================================================================
%  HELPER FUNCTIONS
%  ========================================================================

% Calculate coil parameters from total turns and layers
function result = calc_multilayer(N_total, n_layers, r_inner, wire_d, rho)
    turns_per_layer = N_total / n_layers;
    l = turns_per_layer * wire_d;       % Axial length [m]
    w = n_layers * wire_d;              % Radial build [m]
    r_mean = r_inner + w/2;             % Mean radius [m]
    
    % Inductance using Wheeler formula
    L = (0.8 * (r_mean*100)^2 * N_total^2) / ...
        (6*(r_mean*100) + 9*(l*100) + 10*(w*100)) * 1e-6;
    
    % Wire length (sum of all layers)
    wire_len = 0;
    for layer = 1:n_layers
        r_layer = r_inner + (layer - 0.5) * wire_d;
        wire_len = wire_len + turns_per_layer * 2 * pi * r_layer;
    end
    
    % DC resistance
    area = pi * (wire_d/2)^2;
    R_dc = rho * wire_len / area;
    
    result.N = N_total;
    result.n_layers = n_layers;
    result.turns_per_layer = turns_per_layer;
    result.l = l;
    result.w = w;
    result.r_mean = r_mean;
    result.L = L;
    result.wire_len = wire_len;
    result.R_dc = R_dc;
end

% Find turns needed for target inductance
function N = find_turns(L_target, n_layers, r_inner, wire_d, rho)
    N = 100;  % Initial guess
    for iter = 1:100
        result = calc_multilayer(N, n_layers, r_inner, wire_d, rho);
        L = result.L;
        if L <= 0
            break;
        end
        N_new = N * sqrt(L_target / L);
        if abs(N_new - N) < 0.5
            break;
        end
        N = N_new;
    end
    N = round(N);
end

%% ========================================================================
%  TX COIL CALCULATIONS - READY TO IMPLEMENT
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       TX COIL - READY TO IMPLEMENT                              ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

TX.r_inner = TX.diameter / 2;

% Find turns needed
TX.N = find_turns(TX.L_target, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);

% Calculate all parameters
TX.result = calc_multilayer(TX.N, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);

% Electrical characteristics
TX.X_L = 2*pi*f_tx*TX.result.L;
TX.Z = sqrt(TX.result.R_dc^2 + TX.X_L^2);
TX.I = V_supply / TX.Z;
TX.Q = TX.X_L / TX.result.R_dc;

fprintf('Design Parameters:\n');
fprintf('  Form diameter:      %.0f mm\n', TX.diameter*1000);
fprintf('  Wire diameter:      %.2f mm (AWG 24)\n', TX.wire_diameter*1000);
fprintf('  Target inductance:  %.1f mH\n', TX.L_target*1000);
fprintf('  Number of layers:   %d\n', TX.n_layers);
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  TX COIL WINDING SPECIFICATION - FINAL                        │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Total turns:       %4d turns                                 │\n', TX.N);
fprintf('│  Turns per layer:   %4.0f turns                                 │\n', TX.result.turns_per_layer);
fprintf('│  Number of layers:  %4d layers                                │\n', TX.n_layers);
fprintf('│  Axial length:      %5.1f mm                                   │\n', TX.result.l*1000);
fprintf('│  Radial build:      %5.1f mm                                   │\n', TX.result.w*1000);
fprintf('│  Winding direction: CLOCKWISE                                 │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Calculated Results:\n');
fprintf('  Inductance:         %.2f mH\n', TX.result.L*1000);
fprintf('  Wire length:        %.1f m\n', TX.result.wire_len);
fprintf('  DC Resistance:      %.2f Ω\n', TX.result.R_dc);
fprintf('\n');

fprintf('Electrical @ %d Hz, %.1f V:\n', f_tx, V_supply);
fprintf('  Reactance (X_L):    %.1f Ω\n', TX.X_L);
fprintf('  Impedance (Z):      %.1f Ω\n', TX.Z);
fprintf('  Current:            %.1f mA\n', TX.I*1000);
fprintf('  Q factor:           %.1f\n', TX.Q);
fprintf('\n');

%% ========================================================================
%  TX RESONANCE CAPACITOR
%  ========================================================================
%
%  Purpose: LC tank converts square wave from MCU into sine wave
%           - MCU outputs 2kHz square wave (contains harmonics)
%           - LC tank filters out harmonics, passes only fundamental
%           - Result: clean sine wave for accurate phase detection
%

TX.C_calc = 1 / (4*pi^2*f_tx^2*TX.result.L);  % Calculated capacitance
TX.C_std = 470e-9;  % Standard value (470 nF)
TX.f_resonant = 1 / (2*pi*sqrt(TX.result.L * TX.C_std));

fprintf('Resonance Capacitor:\n');
fprintf('  Calculated C:       %.0f nF\n', TX.C_calc*1e9);
fprintf('  Standard value:     %.0f nF (film capacitor, ≥25V)\n', TX.C_std*1e9);
fprintf('  Actual f_resonant:  %.0f Hz\n', TX.f_resonant);
fprintf('\n');
fprintf('  Circuit: Series LC tank (C in series with L)\n');
fprintf('  Purpose: Converts square wave → sine wave\n');
fprintf('\n');

%% ========================================================================
%  RX COIL CALCULATIONS - PLACEHOLDER
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       RX COIL - PLACEHOLDER (update parameters above)           ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

RX.r_inner = RX.diameter / 2;

% Find turns needed
RX.N = find_turns(RX.L_target, RX.n_layers, RX.r_inner, RX.wire_diameter, RX.wire_rho);

% Calculate all parameters
RX.result = calc_multilayer(RX.N, RX.n_layers, RX.r_inner, RX.wire_diameter, RX.wire_rho);

% Electrical characteristics
RX.X_L = 2*pi*f_tx*RX.result.L;
RX.Z = sqrt(RX.result.R_dc^2 + RX.X_L^2);
RX.Q = RX.X_L / RX.result.R_dc;

fprintf('*** PLACEHOLDER VALUES - UPDATE WHEN DIMENSIONS KNOWN ***\n\n');

fprintf('Current Parameters (TBD):\n');
fprintf('  Form diameter:      %.0f mm\n', RX.diameter*1000);
fprintf('  Wire diameter:      %.2f mm (AWG 28)\n', RX.wire_diameter*1000);
fprintf('  Target inductance:  %.1f mH (requirement 16: >= 10 mH)\n', RX.L_target*1000);
fprintf('  Number of layers:   %d\n', RX.n_layers);
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  RX COIL WINDING SPECIFICATION - PLACEHOLDER                  │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Total turns:       %4d turns                                 │\n', RX.N);
fprintf('│  Turns per layer:   %4.0f turns                                 │\n', RX.result.turns_per_layer);
fprintf('│  Number of layers:  %4d layers                                │\n', RX.n_layers);
fprintf('│  Axial length:      %5.1f mm                                   │\n', RX.result.l*1000);
fprintf('│  Radial build:      %5.1f mm                                   │\n', RX.result.w*1000);
fprintf('│  Winding direction: COUNTER-CLOCKWISE                         │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Calculated Results:\n');
fprintf('  Inductance:         %.2f mH\n', RX.result.L*1000);
fprintf('  Wire length:        %.1f m\n', RX.result.wire_len);
fprintf('  DC Resistance:      %.2f Ω\n', RX.result.R_dc);
fprintf('  Q factor @ 2kHz:    %.1f\n', RX.Q);
fprintf('\n');

% RX Resonance capacitor (parallel configuration)
RX.C_calc = 1 / (4*pi^2*f_tx^2*RX.result.L);
RX.C_std = 680e-9;  % Standard value
RX.f_resonant = 1 / (2*pi*sqrt(RX.result.L * RX.C_std));

fprintf('Resonance Capacitor (parallel):\n');
fprintf('  Calculated C:       %.0f nF\n', RX.C_calc*1e9);
fprintf('  Standard value:     %.0f nF\n', RX.C_std*1e9);
fprintf('  Actual f_resonant:  %.0f Hz\n', RX.f_resonant);
fprintf('  Purpose: Bandpass filter + voltage gain at resonance\n');
fprintf('\n');

%% ========================================================================
%  BUCKING COIL CALCULATIONS - PLACEHOLDER
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║     BUCKING COIL - PLACEHOLDER (update parameters above)        ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

Bucking.r_inner = Bucking.diameter / 2;

% Calculate parameters for starting turns
Bucking.result = calc_multilayer(Bucking.N_start, Bucking.n_layers, Bucking.r_inner, Bucking.wire_diameter, Bucking.wire_rho);

fprintf('*** PLACEHOLDER VALUES - UPDATE WHEN DIMENSIONS KNOWN ***\n\n');

fprintf('Current Parameters (TBD):\n');
fprintf('  Form diameter:      %.0f mm (between TX and RX)\n', Bucking.diameter*1000);
fprintf('  Wire diameter:      %.2f mm\n', Bucking.wire_diameter*1000);
fprintf('  Starting turns:     %d (ADJUST EMPIRICALLY)\n', Bucking.N_start);
fprintf('  Number of layers:   %d\n', Bucking.n_layers);
fprintf('  Winding direction:  COUNTER-CLOCKWISE (same as RX)\n');
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  BUCKING COIL WINDING SPECIFICATION - PLACEHOLDER             │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Starting turns:   %4d turns  (adjust 30-50 for null)        │\n', Bucking.N_start);
fprintf('│  Turns per layer:  %4.0f turns                                 │\n', Bucking.result.turns_per_layer);
fprintf('│  Number of layers: %4d layers                                │\n', Bucking.n_layers);
fprintf('│  Axial length:     %5.1f mm                                   │\n', Bucking.result.l*1000);
fprintf('│  Radial build:     %5.1f mm                                   │\n', Bucking.result.w*1000);
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Calculated Results:\n');
fprintf('  Inductance:         %.2f mH\n', Bucking.result.L*1000);
fprintf('  Wire length:        %.1f m\n', Bucking.result.wire_len);
fprintf('  DC Resistance:      %.2f Ω\n', Bucking.result.R_dc);
fprintf('\n');

%% ========================================================================
%  DESIGN SUMMARY
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║                     DESIGN SUMMARY                               ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('┌─────────────────┬─────────────┬─────────────┬─────────────┐\n');
fprintf('│ Parameter       │   TX Coil   │   RX Coil   │   Bucking   │\n');
fprintf('│                 │   (FINAL)   │   (TBD)     │   (TBD)     │\n');
fprintf('├─────────────────┼─────────────┼─────────────┼─────────────┤\n');
fprintf('│ Form diameter   │   %3.0f mm    │    %2.0f mm    │   %3.0f mm    │\n', TX.diameter*1000, RX.diameter*1000, Bucking.diameter*1000);
fprintf('│ Wire diameter   │  %.2f mm    │  %.2f mm    │  %.2f mm    │\n', TX.wire_diameter*1000, RX.wire_diameter*1000, Bucking.wire_diameter*1000);
fprintf('│ Total turns     │   %3d turns │   %3d turns │    %2d turns │\n', TX.N, RX.N, Bucking.N_start);
fprintf('│ Turns per layer │    %2.0f turns │    %2.0f turns │    %2.0f turns │\n', TX.result.turns_per_layer, RX.result.turns_per_layer, Bucking.result.turns_per_layer);
fprintf('│ Layers          │     %d       │     %d       │     %d       │\n', TX.n_layers, RX.n_layers, Bucking.n_layers);
fprintf('│ Axial length    │   %4.1f mm   │   %4.1f mm   │   %4.1f mm   │\n', TX.result.l*1000, RX.result.l*1000, Bucking.result.l*1000);
fprintf('│ Radial build    │    %3.1f mm   │    %3.1f mm   │    %3.1f mm   │\n', TX.result.w*1000, RX.result.w*1000, Bucking.result.w*1000);
fprintf('│ Inductance      │   %4.1f mH   │   %4.1f mH   │   %4.2f mH  │\n', TX.result.L*1000, RX.result.L*1000, Bucking.result.L*1000);
fprintf('│ DC Resistance   │   %4.2f Ω    │   %4.2f Ω    │   %4.2f Ω    │\n', TX.result.R_dc, RX.result.R_dc, Bucking.result.R_dc);
fprintf('│ Wire length     │   %4.1f m    │   %4.1f m    │   %4.1f m    │\n', TX.result.wire_len, RX.result.wire_len, Bucking.result.wire_len);
fprintf('├─────────────────┼─────────────┴─────────────┴─────────────┤\n');
fprintf('│ Winding dir.    │  CLOCKWISE    COUNTER-CW    COUNTER-CW  │\n');
fprintf('└─────────────────┴─────────────────────────────────────────┘\n');
fprintf('\n');
fprintf('Total wire needed: TX=%.0fm + RX=%.0fm + Bucking=%.0fm = %.0fm (+ spare)\n', ...
    TX.result.wire_len, RX.result.wire_len, Bucking.result.wire_len, ...
    TX.result.wire_len + RX.result.wire_len + Bucking.result.wire_len);

%% ========================================================================
%  TX COIL WINDING INSTRUCTIONS - READY TO IMPLEMENT
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║            TX COIL WINDING INSTRUCTIONS - FINAL                  ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('TX COIL (%.0f mm diameter form) - Wind CLOCKWISE:\n\n', TX.diameter*1000);
fprintf('  Materials needed:\n');
fprintf('    - Plastic cylinder/form: %.0f mm diameter\n', TX.diameter*1000);
fprintf('    - Copper wire: %.2f mm (AWG 24), %.0f m length\n', TX.wire_diameter*1000, TX.result.wire_len + 5);
fprintf('    - Electrical tape for securing layers\n');
fprintf('\n');
fprintf('  Winding procedure:\n');
fprintf('    1. Mark starting point on the %.0f mm form\n', TX.diameter*1000);
fprintf('    2. Wind %.0f turns CLOCKWISE, close-wound (layer 1)\n', TX.result.turns_per_layer);
fprintf('    3. Apply thin tape to secure\n');
fprintf('    4. Wind %.0f turns back over layer 1 (layer 2)\n', TX.result.turns_per_layer);
fprintf('    5. Apply tape, then wind %.0f turns for layer 3\n', TX.result.turns_per_layer);
fprintf('    6. Total: %d turns in %d layers\n', TX.N, TX.n_layers);
fprintf('\n');
fprintf('  Final dimensions:\n');
fprintf('    - Coil length (axial):  %.1f mm\n', TX.result.l*1000);
fprintf('    - Coil thickness:       %.1f mm\n', TX.result.w*1000);
fprintf('\n');
fprintf('  Verification targets:\n');
fprintf('    - Inductance: %.1f mH ± 20%% (%.1f - %.1f mH)\n', ...
    TX.result.L*1000, TX.result.L*1000*0.8, TX.result.L*1000*1.2);
fprintf('    - DC Resistance: < %.1f Ω\n', TX.result.R_dc * 1.5);
fprintf('    - Current @ %.1fV: %.0f - %.0f mA\n', V_supply, TX.I*1000*0.8, TX.I*1000*1.2);
fprintf('\n');
fprintf('  Resonance capacitor:\n');
fprintf('    - Value: 470 nF (film capacitor)\n');
fprintf('    - Voltage rating: >= 25V\n');
fprintf('    - Connection: Series with TX coil\n');
fprintf('    - Purpose: Converts square wave to sine wave\n');
fprintf('\n');

%% ========================================================================
%  RX AND BUCKING COIL NOTES
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║          RX AND BUCKING COIL - AWAITING DIMENSIONS               ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('RX Coil:\n');
fprintf('  - Target inductance: >= 10 mH (requirement 16)\n');
fprintf('  - Winding direction: COUNTER-CLOCKWISE\n');
fprintf('  - Can use thinner wire (AWG 26-30) since no significant current\n');
fprintf('  - Update RX.diameter, RX.wire_diameter, RX.n_layers above\n');
fprintf('\n');

fprintf('Bucking Coil:\n');
fprintf('  - Purpose: Cancel direct TX field at RX location\n');
fprintf('  - Position: Between TX and RX (diameter between the two)\n');
fprintf('  - Winding direction: COUNTER-CLOCKWISE (same as RX)\n');
fprintf('  - Connection: In series with RX coil\n');
fprintf('  - Turns must be adjusted empirically for null (30-50 range)\n');
fprintf('  - Update Bucking.diameter, Bucking.N_start above\n');
fprintf('\n');

fprintf('Bucking adjustment procedure:\n');
fprintf('  1. Wind all three coils\n');
fprintf('  2. Connect RX and Bucking in series (same polarity)\n');
fprintf('  3. Power TX with 2 kHz signal\n');
fprintf('  4. Measure RX+Bucking output on oscilloscope\n');
fprintf('  5. Adjust bucking turns until signal minimized (null)\n');
fprintf('\n');

%% ========================================================================
%  QUICK REFERENCE TABLES
%  ========================================================================

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║                    QUICK REFERENCE TABLES                        ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('TX Coil (250 mm, 3 layers, AWG 24) - Turns vs Inductance:\n\n');
fprintf('  Turns    Inductance\n');
fprintf('  -----    ----------\n');
for N_test = round(TX.N * [0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3])
    result = calc_multilayer(N_test, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    marker = '';
    if N_test == TX.N
        marker = ' ← Design';
    end
    fprintf('  %4d     %6.2f mH%s\n', N_test, result.L*1000, marker);
end
fprintf('\n');

fprintf('RX Coil wire options (80mm diameter, 10mH, ~20mm tall):\n\n');
fprintf('  AWG    Wire ⌀     Layers   Turns   Height    R_dc\n');
fprintf('  ---    ------     ------   -----   ------    ----\n');
fprintf('  24     0.52 mm       5      180     19 mm    3.7 Ω\n');
fprintf('  26     0.40 mm       4      177     18 mm    6.1 Ω\n');
fprintf('  28     0.32 mm       4      169     14 mm    9.0 Ω\n');
fprintf('  30     0.25 mm       2      188     24 mm   16.3 Ω\n');
fprintf('\n');

%% ========================================================================
%  PLOTS
%  ========================================================================

figure('Name', 'TX Coil Design - Ready to Implement', 'Position', [100, 100, 800, 400]);

% TX turns vs inductance
subplot(1,2,1);
N_range = 60:5:150;
L_range_TX = zeros(size(N_range));
for i = 1:length(N_range)
    result = calc_multilayer(N_range(i), TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    L_range_TX(i) = result.L * 1000;
end
plot(N_range, L_range_TX, 'b-', 'LineWidth', 2);
hold on;
plot(TX.N, TX.result.L*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
yline(TX.L_target*1000, 'g--', 'Target 15 mH', 'LineWidth', 1.5);
xlabel('Total Turns');
ylabel('Inductance [mH]');
title(sprintf('TX Coil (250mm, %d layers, AWG 24)', TX.n_layers));
grid on;
legend('Wheeler Formula', sprintf('%d turns = %.1f mH', TX.N, TX.result.L*1000), 'Location', 'northwest');

% TX current vs inductance
subplot(1,2,2);
L_range = 5:1:25;
I_range = zeros(size(L_range));
for i = 1:length(L_range)
    X_L = 2*pi*f_tx*L_range(i)/1000;
    Z = sqrt(TX.result.R_dc^2 + X_L^2);
    I_range(i) = V_supply / Z * 1000;
end
plot(L_range, I_range, 'b-', 'LineWidth', 2);
hold on;
plot(TX.result.L*1000, TX.I*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
xlabel('Inductance [mH]');
ylabel('Current [mA]');
title(sprintf('TX Current @ %.1fV, %d Hz', V_supply, f_tx));
grid on;
legend('Current vs L', sprintf('%.1f mH = %.1f mA', TX.result.L*1000, TX.I*1000), 'Location', 'northeast');

sgtitle('TX Coil Design - VLF Metal Detector', 'FontSize', 14, 'FontWeight', 'bold');

fprintf('\nFigure generated.\n');
fprintf('\n');
fprintf('*** TX COIL IS READY TO IMPLEMENT ***\n');
fprintf('*** Update RX and Bucking parameters when dimensions are known ***\n');
