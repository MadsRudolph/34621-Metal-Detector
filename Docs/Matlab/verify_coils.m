%% Coil Verification Tool
% Interactive script to compare LCR measurements against Wheeler theory
% DTU 34621 VLF Metal Detector Project
%
% Usage: Run this script and follow the prompts to enter your measurements

clear; clc;

fprintf('\n');
fprintf('══════════════════════════════════════════════════════════════════\n');
fprintf('           COIL VERIFICATION TOOL\n');
fprintf('     Compare LCR measurements vs Wheeler theory\n');
fprintf('══════════════════════════════════════════════════════════════════\n\n');

%% Wheeler Formula Functions

function result = calc_coil(N_total, n_layers, r_inner, wire_d, rho)
    turns_per_layer = N_total / n_layers;
    l = turns_per_layer * wire_d;
    w = n_layers * wire_d;
    r_mean = r_inner + w/2;

    r_cm = r_mean * 100;
    l_cm = l * 100;
    w_cm = w * 100;

    if n_layers == 1
        L = (0.394 * r_cm^2 * N_total^2) / (9*r_cm + 10*l_cm) * 1e-6;
        result.formula = 'Wheeler Single-Layer';
    else
        L = (0.315 * r_cm^2 * N_total^2) / (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
        result.formula = 'Wheeler Multilayer';
    end

    wire_len = 0;
    for layer = 1:n_layers
        r_layer = r_inner + (layer - 0.5) * wire_d;
        wire_len = wire_len + turns_per_layer * 2 * pi * r_layer;
    end

    area = pi * (wire_d/2)^2;
    R_dc = rho * wire_len / area;

    result.N = N_total;
    result.n_layers = n_layers;
    result.L = L;
    result.R_dc = R_dc;
    result.wire_len = wire_len;
end

%% Default Values (from prototype)

defaults.TX.turns = 70;
defaults.TX.layers = 2;
defaults.TX.diameter = 200;
defaults.TX.wire_d = 0.52;

defaults.RX.turns = 440;
defaults.RX.layers = 4;
defaults.RX.diameter = 50;
defaults.RX.wire_d = 0.15;

defaults.Bucking.turns = 35;
defaults.Bucking.layers = 1;
defaults.Bucking.diameter = 60;
defaults.Bucking.wire_d = 0.52;

rho_copper = 1.68e-8;

%% User Input - Coil Specifications

fprintf('Enter coil specifications (press Enter for default value)\n');
fprintf('─────────────────────────────────────────────────────────\n\n');

% TX Coil
fprintf('TX COIL:\n');
TX.turns = input(sprintf('  Turns [%d]: ', defaults.TX.turns));
if isempty(TX.turns), TX.turns = defaults.TX.turns; end

TX.layers = input(sprintf('  Layers [%d]: ', defaults.TX.layers));
if isempty(TX.layers), TX.layers = defaults.TX.layers; end

TX.diameter = input(sprintf('  Diameter in mm [%d]: ', defaults.TX.diameter));
if isempty(TX.diameter), TX.diameter = defaults.TX.diameter; end

TX.wire_d = input(sprintf('  Wire diameter in mm [%.2f]: ', defaults.TX.wire_d));
if isempty(TX.wire_d), TX.wire_d = defaults.TX.wire_d; end

fprintf('\n');

% RX Coil
fprintf('RX COIL:\n');
RX.turns = input(sprintf('  Turns [%d]: ', defaults.RX.turns));
if isempty(RX.turns), RX.turns = defaults.RX.turns; end

RX.layers = input(sprintf('  Layers [%d]: ', defaults.RX.layers));
if isempty(RX.layers), RX.layers = defaults.RX.layers; end

RX.diameter = input(sprintf('  Diameter in mm [%d]: ', defaults.RX.diameter));
if isempty(RX.diameter), RX.diameter = defaults.RX.diameter; end

RX.wire_d = input(sprintf('  Wire diameter in mm [%.2f]: ', defaults.RX.wire_d));
if isempty(RX.wire_d), RX.wire_d = defaults.RX.wire_d; end

fprintf('\n');

% Bucking Coil
fprintf('BUCKING COIL:\n');
Bucking.turns = input(sprintf('  Turns [%d]: ', defaults.Bucking.turns));
if isempty(Bucking.turns), Bucking.turns = defaults.Bucking.turns; end

Bucking.layers = input(sprintf('  Layers [%d]: ', defaults.Bucking.layers));
if isempty(Bucking.layers), Bucking.layers = defaults.Bucking.layers; end

Bucking.diameter = input(sprintf('  Diameter in mm [%d]: ', defaults.Bucking.diameter));
if isempty(Bucking.diameter), Bucking.diameter = defaults.Bucking.diameter; end

Bucking.wire_d = input(sprintf('  Wire diameter in mm [%.2f]: ', defaults.Bucking.wire_d));
if isempty(Bucking.wire_d), Bucking.wire_d = defaults.Bucking.wire_d; end

fprintf('\n');

%% User Input - LCR Measurements

fprintf('Enter LCR meter measurements\n');
fprintf('─────────────────────────────────────────────────────────\n\n');

fprintf('Measurement frequency in Hz [1000]: ');
meas_freq = input('');
if isempty(meas_freq), meas_freq = 1000; end

fprintf('\n');

fprintf('How did you measure TX and Bucking coils?\n');
fprintf('  1. Together in series\n');
fprintf('  2. Separately\n');
meas_mode = input('Choice [1]: ');
if isempty(meas_mode), meas_mode = 1; end

fprintf('\n');

if meas_mode == 1
    Meas.TX_Bucking_L = input('TX + Bucking inductance in mH: ') * 1e-3;
    Meas.TX_Bucking_Z = input('TX + Bucking impedance in Ohm (or Enter to skip): ');
    if isempty(Meas.TX_Bucking_Z), Meas.TX_Bucking_Z = NaN; end
    Meas.TX_L = NaN;
    Meas.Bucking_L = NaN;
else
    Meas.TX_L = input('TX inductance in mH: ') * 1e-3;
    Meas.Bucking_L = input('Bucking inductance in mH: ') * 1e-3;
    Meas.TX_Bucking_L = Meas.TX_L + Meas.Bucking_L;
    Meas.TX_Bucking_Z = NaN;
end

fprintf('\n');

Meas.RX_L = input('RX inductance in mH: ') * 1e-3;
Meas.RX_Z = input('RX impedance in Ohm (or Enter to skip): ');
if isempty(Meas.RX_Z), Meas.RX_Z = NaN; end

fprintf('\n');

tolerance = input('Error tolerance %% [10]: ');
if isempty(tolerance), tolerance = 10; end

%% Calculate Theoretical Values

TX.result = calc_coil(TX.turns, TX.layers, TX.diameter/2*1e-3, TX.wire_d*1e-3, rho_copper);
RX.result = calc_coil(RX.turns, RX.layers, RX.diameter/2*1e-3, RX.wire_d*1e-3, rho_copper);
Bucking.result = calc_coil(Bucking.turns, Bucking.layers, Bucking.diameter/2*1e-3, Bucking.wire_d*1e-3, rho_copper);

Theory.TX_Bucking_L = TX.result.L + Bucking.result.L;
Theory.TX_Bucking_R = TX.result.R_dc + Bucking.result.R_dc;
Theory.RX_L = RX.result.L;
Theory.RX_R = RX.result.R_dc;

% Extract DC resistance from impedance if available
if ~isnan(Meas.RX_Z)
    X_L = 2 * pi * meas_freq * Meas.RX_L;
    if Meas.RX_Z > X_L
        Meas.RX_R = sqrt(Meas.RX_Z^2 - X_L^2);
    else
        Meas.RX_R = NaN;
    end
else
    Meas.RX_R = NaN;
end

%% Display Results

fprintf('\n');
fprintf('══════════════════════════════════════════════════════════════════\n');
fprintf('                    VERIFICATION RESULTS\n');
fprintf('══════════════════════════════════════════════════════════════════\n\n');

fprintf('┌─────────────────────────────────────────────────────────────────┐\n');
fprintf('│  COIL SPECIFICATIONS                                           │\n');
fprintf('├─────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Coil      Turns  Layers  Diameter  Wire      Theoretical L   │\n');
fprintf('├─────────────────────────────────────────────────────────────────┤\n');
fprintf('│  TX        %4d     %d      %3d mm   %.2f mm    %5.2f mH       │\n', ...
    TX.turns, TX.layers, TX.diameter, TX.wire_d, TX.result.L*1000);
fprintf('│  Bucking   %4d     %d       %2d mm   %.2f mm    %5.3f mH       │\n', ...
    Bucking.turns, Bucking.layers, Bucking.diameter, Bucking.wire_d, Bucking.result.L*1000);
fprintf('│  RX        %4d     %d       %2d mm   %.2f mm    %5.1f mH       │\n', ...
    RX.turns, RX.layers, RX.diameter, RX.wire_d, RX.result.L*1000);
fprintf('└─────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

% Calculate errors
err_TX_Buck_L = (Theory.TX_Bucking_L - Meas.TX_Bucking_L) / Meas.TX_Bucking_L * 100;
err_RX_L = (Theory.RX_L - Meas.RX_L) / Meas.RX_L * 100;
if ~isnan(Meas.RX_R)
    err_RX_R = (Theory.RX_R - Meas.RX_R) / Meas.RX_R * 100;
else
    err_RX_R = NaN;
end

% Pass/fail
function str = status(err, tol)
    if abs(err) <= tol
        str = 'PASS';
    else
        str = 'FAIL';
    end
end

fprintf('┌─────────────────────────────────────────────────────────────────┐\n');
fprintf('│  COMPARISON (tolerance: +/- %d%%)                                │\n', tolerance);
fprintf('├─────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Parameter          Theoretical   Measured     Error   Status  │\n');
fprintf('├─────────────────────────────────────────────────────────────────┤\n');

if meas_mode == 1
    fprintf('│  TX+Bucking L       %5.2f mH      %5.2f mH    %+5.1f%%   %s   │\n', ...
        Theory.TX_Bucking_L*1000, Meas.TX_Bucking_L*1000, err_TX_Buck_L, status(err_TX_Buck_L, tolerance));
else
    err_TX = (TX.result.L - Meas.TX_L) / Meas.TX_L * 100;
    err_Buck = (Bucking.result.L - Meas.Bucking_L) / Meas.Bucking_L * 100;
    fprintf('│  TX L               %5.2f mH      %5.2f mH    %+5.1f%%   %s   │\n', ...
        TX.result.L*1000, Meas.TX_L*1000, err_TX, status(err_TX, tolerance));
    fprintf('│  Bucking L          %5.3f mH      %5.3f mH    %+5.1f%%   %s   │\n', ...
        Bucking.result.L*1000, Meas.Bucking_L*1000, err_Buck, status(err_Buck, tolerance));
end

fprintf('│  RX L               %5.1f mH      %5.1f mH    %+5.1f%%   %s   │\n', ...
    Theory.RX_L*1000, Meas.RX_L*1000, err_RX_L, status(err_RX_L, tolerance));

if ~isnan(Meas.RX_R)
    fprintf('│  RX R (from Z)      %5.1f Ohm     %5.1f Ohm   %+5.1f%%   %s   │\n', ...
        Theory.RX_R, Meas.RX_R, err_RX_R, status(err_RX_R, tolerance));
end

fprintf('└─────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

% Overall result
all_pass = abs(err_TX_Buck_L) <= tolerance && abs(err_RX_L) <= tolerance;
if ~isnan(err_RX_R)
    all_pass = all_pass && abs(err_RX_R) <= tolerance;
end

if all_pass
    fprintf('══════════════════════════════════════════════════════════════════\n');
    fprintf('  RESULT: ALL TESTS PASSED\n');
    fprintf('  Wheeler formulas match LCR measurements within +/- %d%%\n', tolerance);
    fprintf('══════════════════════════════════════════════════════════════════\n');
else
    fprintf('══════════════════════════════════════════════════════════════════\n');
    fprintf('  RESULT: SOME TESTS FAILED\n');
    fprintf('  Check coil specifications or measurement setup\n');
    fprintf('══════════════════════════════════════════════════════════════════\n');
end

fprintf('\n');

%% Additional Info

fprintf('┌─────────────────────────────────────────────────────────────────┐\n');
fprintf('│  ADDITIONAL CALCULATED VALUES                                  │\n');
fprintf('├─────────────────────────────────────────────────────────────────┤\n');
fprintf('│  TX wire length:       %5.1f m                                 │\n', TX.result.wire_len);
fprintf('│  TX DC resistance:     %5.2f Ohm                               │\n', TX.result.R_dc);
fprintf('│  Bucking wire length:  %5.1f m                                 │\n', Bucking.result.wire_len);
fprintf('│  Bucking DC resistance:%5.2f Ohm                               │\n', Bucking.result.R_dc);
fprintf('│  RX wire length:       %5.1f m                                 │\n', RX.result.wire_len);
fprintf('│  RX DC resistance:     %5.1f Ohm                               │\n', RX.result.R_dc);
fprintf('└─────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');
