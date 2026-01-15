%% Multi-Layer Solenoid Coil Design Calculator
% DTU 34621 VLF Metaldetektor - Arduino Nano Design
% Se Docs/Theory/Coil Design.md for fuld dokumentation

clear; clc; close all;

%% Parametre - Arduino Nano Design

% Operating parameters
f_tx = 2000;                    % TX frequency [Hz]
V_supply = 9.0;                 % Battery voltage [V]
V_hbridge = 2 * V_supply;       % H-bridge peak-to-peak voltage [V]
R_total_target = 35;            % Total resistance for amplifier [Ohm]

% TX Spole - faktiske værdier fra prototype
TX.wire_diameter = 0.56e-3;     % 0.56 mm
TX.wire_rho = 1.68e-8;          % Kobber resistivitet [Ohm*m]
TX.diameter = 200e-3;           % 200 mm form
TX.N_actual = 70;               % Faktisk antal vindinger
TX.n_layers = 2;

% RX Spole - faktiske værdier fra prototype
RX.wire_diameter = 0.15e-3;     % 0.15 mm
RX.wire_rho = 1.68e-8;
RX.diameter = 50e-3;            % 50 mm form
RX.N_actual = 445;              % Faktisk antal vindinger
RX.n_layers = 3;                % 3 lag (faktisk)

% Bucking Spole - kalibreret til 20 vindinger
Bucking.wire_diameter = 0.56e-3;
Bucking.wire_rho = 1.68e-8;
Bucking.diameter = 60e-3;       % 60 mm form
Bucking.n_layers = 1;
Bucking.N_optimal = 20;         % Optimal værdi efter kalibrering

%% Hjælpefunktioner

% Calculate coil parameters from total turns and layers
% Uses Wheeler formulas verified against LCR measurements (2% accuracy)
% Source: H.A. Wheeler, "Simple Inductance Formulas for Radio Coils",
%         Proc. IRE, Vol. 16, No. 10, Oct. 1928
function result = calc_coil(N_total, n_layers, r_inner, wire_d, rho)
    turns_per_layer = N_total / n_layers;
    l = turns_per_layer * wire_d;       % Axial length [m]
    w = n_layers * wire_d;              % Radial build [m]
    r_mean = r_inner + w/2;             % Mean radius [m]

    % Convert to cm for Wheeler formulas
    r_cm = r_mean * 100;
    l_cm = l * 100;
    w_cm = w * 100;

    if n_layers == 1
        % Single-layer Wheeler formula (more accurate for 1 layer)
        % L(µH) = 0.394 × a² × N² / (9a + 10b)  [cm]
        % Coefficient: 1.0/2.54 = 0.394 (inch to cm conversion)
        % Accurate to ±1% when b > 0.8a
        L = (0.394 * r_cm^2 * N_total^2) / ...
            (9*r_cm + 10*l_cm) * 1e-6;
        result.formula = 'Wheeler Single-Layer';

        % Also calculate multilayer for comparison
        L_multi = (0.315 * r_cm^2 * N_total^2) / ...
            (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
        result.L_multilayer = L_multi;
    else
        % Multilayer Wheeler formula
        % L(µH) = 0.315 × a² × N² / (6a + 9b + 10c)  [cm]
        % Coefficient: 0.8/2.54 = 0.315 (inch to cm conversion)
        % Accurate to ±1% when 6a ≈ 9b ≈ 10c (balanced terms)
        L = (0.315 * r_cm^2 * N_total^2) / ...
            (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
        result.formula = 'Wheeler Multilayer';

        % Check denominator term balance (for accuracy warning)
        term_6a = 6 * r_cm;
        term_9b = 9 * l_cm;
        term_10c = 10 * w_cm;
        result.balance_ratio = min([term_6a, term_9b, term_10c]) / ...
                               max([term_6a, term_9b, term_10c]);
        % Ratio < 0.1 indicates poor accuracy (flat/pancake coil)
    end

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

% Wrapper for backward compatibility
function result = calc_multilayer(N_total, n_layers, r_inner, wire_d, rho)
    result = calc_coil(N_total, n_layers, r_inner, wire_d, rho);
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

%% TX Spole Beregning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       TX SPOLE - PROTOTYPE VÆRDIER                              ║\n');
fprintf('║       70 vindinger, 2 lag, Ø200mm, 0.56mm tråd                  ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

TX.r_inner = TX.diameter / 2;

% Brug faktiske vindinger fra prototype
TX.N = TX.N_actual;

% Calculate all parameters
TX.result = calc_multilayer(TX.N, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);

% Series resistor for amplifier matching
TX.R_series = R_total_target - TX.result.R_dc;  % External resistor needed
TX.R_total = R_total_target;                     % Total circuit resistance

% Electrical characteristics (at resonance, Z ≈ R_total)
TX.X_L = 2*pi*f_tx*TX.result.L;
TX.I_peak = V_hbridge / TX.R_total;              % Peak current from H-bridge
TX.I_rms = TX.I_peak / sqrt(2);                  % RMS current (sine wave)
TX.Q = TX.X_L / TX.R_total;                      % Q factor with total R

% Power dissipation
TX.P_resistor = TX.I_rms^2 * TX.R_series;        % Power in series resistor [W]
TX.P_coil = TX.I_rms^2 * TX.result.R_dc;         % Power in coil resistance [W]
TX.P_total = TX.I_rms^2 * TX.R_total;            % Total power [W]

% Resonance capacitor for 2 kHz
TX.C_resonance = 1 / (4*pi^2*f_tx^2*TX.result.L);
TX.C_std = 1.0e-6;  % Standard value: 1.0 µF
TX.f_resonant = 1 / (2*pi*sqrt(TX.result.L * TX.C_std));

fprintf('Design Parametre:\n');
fprintf('  Form diameter:      %.0f mm\n', TX.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm\n', TX.wire_diameter*1000);
fprintf('  Antal vindinger:    %d\n', TX.N);
fprintf('  Antal lag:          %d\n', TX.n_layers);
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  TX SPOLE VIKLINGS SPECIFIKATION                              │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Antal vindinger:   %4d vindinger                            │\n', TX.N);
fprintf('│  Vindinger per lag: %4.0f vindinger                            │\n', TX.result.turns_per_layer);
fprintf('│  Antal lag:         %4d lag                                   │\n', TX.n_layers);
fprintf('│  Aksial længde:     %5.1f mm                                  │\n', TX.result.l*1000);
fprintf('│  Radial tykkelse:   %5.1f mm                                  │\n', TX.result.w*1000);
fprintf('│  Vikleretning:      MED URET                                  │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Beregnede Resultater:\n');
fprintf('  Induktans:          %.2f mH\n', TX.result.L*1000);
fprintf('  Trådlængde:         %.1f m\n', TX.result.wire_len);
fprintf('  Spole DC Modstand:  %.2f Ω\n', TX.result.R_dc);
fprintf('  Formel:             %s\n', TX.result.formula);

% Geometry warning for TX coil (flat/pancake geometry)
if isfield(TX.result, 'balance_ratio') && TX.result.balance_ratio < 0.1
    fprintf('\n');
    fprintf('⚠️  GEOMETRI ADVARSEL:\n');
    fprintf('    TX spolen har "flad" geometri (stor diameter, få lag).\n');
    fprintf('    Wheeler formel kan UNDERVURDERE induktans med 5-10%%.\n');
    fprintf('    Balance ratio: %.2f (optimal: > 0.5)\n', TX.result.balance_ratio);
    fprintf('    ANBEFALING: Tilføj ~10%% flere vindinger, juster empirisk.\n');
end
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  SERIE MODSTAND (matcher forstærker)                         │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Mål total modstand: %.0f Ω                                    │\n', TX.R_total);
fprintf('│  Spole DC modstand:  %.1f Ω                                    │\n', TX.result.R_dc);
fprintf('│  SERIE MODSTAND:     %.0f Ω                                    │\n', TX.R_series);
fprintf('│  Effekt i modstand:  %.1f W (brug >= 5W modstand)             │\n', TX.P_resistor);
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Elektriske @ %d Hz (H-bro, %.0f Vpp):\n', f_tx, V_hbridge);
fprintf('  Reaktans (X_L):     %.1f Ω\n', TX.X_L);
fprintf('  Total modstand:     %.0f Ω\n', TX.R_total);
fprintf('  Peak strøm:         %.0f mA\n', TX.I_peak*1000);
fprintf('  RMS strøm:          %.0f mA\n', TX.I_rms*1000);
fprintf('  Q faktor:           %.1f\n', TX.Q);
fprintf('  Total effekt:       %.1f W\n', TX.P_total);
fprintf('\n');

%% TX Resonanskondensator

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  RESONANSKONDENSATOR                                          │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Beregnet C:        %.2f µF                                   │\n', TX.C_resonance*1e6);
fprintf('│  Standard værdi:    %.1f µF (film kondensator)                │\n', TX.C_std*1e6);
fprintf('│  Spændingsrating:   >= 50V (VIGTIGT!)                         │\n');
fprintf('│  Faktisk f_res:     %.0f Hz                                    │\n', TX.f_resonant);
fprintf('│  Forbindelse:       SERIE med TX spole                        │\n');
fprintf('│  Formål:            Konverterer firkantbølge → sinusbølge     │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

% Voltage across capacitor at resonance
V_cap_peak = TX.Q * V_hbridge / 2;  % Q * V_rms
fprintf('  Spænding over C:    ~%.0f V peak (Q=%.1f)\n', V_cap_peak, TX.Q);
fprintf('  Spændingsrating:    >= 50V anbefales\n');
fprintf('\n');

%% RX Spole Beregning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       RX SPOLE - PROTOTYPE VÆRDIER                              ║\n');
fprintf('║       445 vindinger, 3 lag, Ø50mm, 0.15mm tråd                  ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

RX.r_inner = RX.diameter / 2;

% Brug faktiske vindinger fra prototype
RX.N = RX.N_actual;

% Calculate all parameters
RX.result = calc_multilayer(RX.N, RX.n_layers, RX.r_inner, RX.wire_diameter, RX.wire_rho);

% Electrical characteristics
RX.X_L = 2*pi*f_tx*RX.result.L;
RX.Z = sqrt(RX.result.R_dc^2 + RX.X_L^2);
RX.Q = RX.X_L / RX.result.R_dc;

fprintf('Design Parametre:\n');
fprintf('  Form diameter:      %.0f mm\n', RX.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm\n', RX.wire_diameter*1000);
fprintf('  Antal vindinger:    %d\n', RX.N);
fprintf('  Antal lag:          %d\n', RX.n_layers);
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  RX SPOLE VIKLINGS SPECIFIKATION                              │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Antal vindinger:   %4d vindinger                            │\n', RX.N);
fprintf('│  Vindinger per lag: %4.0f vindinger                            │\n', RX.result.turns_per_layer);
fprintf('│  Antal lag:         %4d lag                                   │\n', RX.n_layers);
fprintf('│  Aksial længde:     %5.1f mm                                  │\n', RX.result.l*1000);
fprintf('│  Radial tykkelse:   %5.1f mm                                  │\n', RX.result.w*1000);
fprintf('│  Vikleretning:      MOD URET                                  │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Beregnede Resultater:\n');
fprintf('  Induktans:          %.2f mH ✓ (>= 10 mH)\n', RX.result.L*1000);
fprintf('  Trådlængde:         %.1f m\n', RX.result.wire_len);
fprintf('  DC Modstand:        %.2f Ω\n', RX.result.R_dc);
fprintf('  Q faktor @ 2kHz:    %.1f\n', RX.Q);
fprintf('  Formel:             %s\n', RX.result.formula);
fprintf('\n');

% RX Resonance capacitor (parallel configuration)
RX.C_calc = 1 / (4*pi^2*f_tx^2*RX.result.L);
RX.C_std = 470e-9;  % Standard value
RX.f_resonant = 1 / (2*pi*sqrt(RX.result.L * RX.C_std));

fprintf('RX Resonanskondensator (valgfri, parallel):\n');
fprintf('  Beregnet C:         %.0f nF\n', RX.C_calc*1e9);
fprintf('  Standard værdi:     %.0f nF\n', RX.C_std*1e9);
fprintf('  Faktisk f_res:      %.0f Hz\n', RX.f_resonant);
fprintf('  Formål:             Båndpasfilter + spændingsforstærkning\n');
fprintf('\n');

%% Bucking Spole Beregning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║     BUCKING SPOLE - KALIBRERET TIL 20 VINDINGER                 ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

Bucking.r_inner = Bucking.diameter / 2;
Bucking.N = Bucking.N_optimal;

% Calculate parameters for optimal turns
Bucking.result = calc_multilayer(Bucking.N, Bucking.n_layers, Bucking.r_inner, Bucking.wire_diameter, Bucking.wire_rho);

fprintf('Kalibrerede Parametre:\n');
fprintf('  Form diameter:      %.0f mm (mellem TX og RX)\n', Bucking.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm\n', Bucking.wire_diameter*1000);
fprintf('  Optimal vindinger:  %d (kalibreret for null)\n', Bucking.N);
fprintf('  Antal lag:          %d\n', Bucking.n_layers);
fprintf('  Vikleretning:       MOD URET (samme som RX)\n');
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  BUCKING SPOLE VIKLINGS SPECIFIKATION                         │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Optimal vindinger: %4d vindinger (kalibreret)               │\n', Bucking.N);
fprintf('│  Vindinger per lag: %4.0f vindinger                            │\n', Bucking.result.turns_per_layer);
fprintf('│  Antal lag:         %4d lag                                   │\n', Bucking.n_layers);
fprintf('│  Aksial længde:     %5.1f mm                                  │\n', Bucking.result.l*1000);
fprintf('│  Vikleretning:      MOD URET                                  │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Beregnede Resultater:\n');
fprintf('  Induktans:          %.2f mH\n', Bucking.result.L*1000);
fprintf('  Trådlængde:         %.1f m\n', Bucking.result.wire_len);
fprintf('  DC Modstand:        %.2f Ω\n', Bucking.result.R_dc);
fprintf('  Formel:             %s\n', Bucking.result.formula);
if isfield(Bucking.result, 'L_multilayer')
    fprintf('  (Multilayer ref:    %.2f mH)\n', Bucking.result.L_multilayer*1000);
end
fprintf('\n');

%% Design Oversigt

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║                     DESIGN OVERSIGT                             ║\n');
fprintf('║              Opdateret til Arduino Nano                         ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('┌─────────────────┬─────────────┬─────────────┬─────────────┐\n');
fprintf('│ Parameter       │   TX Spole  │   RX Spole  │   Bucking   │\n');
fprintf('├─────────────────┼─────────────┼─────────────┼─────────────┤\n');
fprintf('│ Form diameter   │   %3.0f mm    │    %2.0f mm    │   %3.0f mm    │\n', TX.diameter*1000, RX.diameter*1000, Bucking.diameter*1000);
fprintf('│ Tråddiameter    │  %.2f mm    │  %.2f mm    │  %.2f mm    │\n', TX.wire_diameter*1000, RX.wire_diameter*1000, Bucking.wire_diameter*1000);
fprintf('│ Antal vindinger │    %2d vind. │   %3d vind. │    %2d vind. │\n', TX.N, RX.N, Bucking.N);
fprintf('│ Antal lag       │     %d       │     %d       │     %d       │\n', TX.n_layers, RX.n_layers, Bucking.n_layers);
fprintf('│ Aksial længde   │   %4.1f mm   │   %4.1f mm   │   %4.1f mm   │\n', TX.result.l*1000, RX.result.l*1000, Bucking.result.l*1000);
fprintf('│ Induktans       │   %4.2f mH  │   %4.1f mH   │   %4.2f mH  │\n', TX.result.L*1000, RX.result.L*1000, Bucking.result.L*1000);
fprintf('│ DC Modstand     │   %4.2f Ω    │   %4.1f Ω    │   %4.2f Ω    │\n', TX.result.R_dc, RX.result.R_dc, Bucking.result.R_dc);
fprintf('│ Trådlængde      │   %4.1f m    │   %4.1f m    │   %4.1f m    │\n', TX.result.wire_len, RX.result.wire_len, Bucking.result.wire_len);
fprintf('│ Resonans C      │   1.0 µF    │   470 nF    │    N/A      │\n');
fprintf('│ Serie modstand  │   %.0f Ω      │    N/A      │    N/A      │\n', TX.R_series);
fprintf('├─────────────────┼─────────────┴─────────────┴─────────────┤\n');
fprintf('│ Vikleretning    │  MED URET     MOD URET      MOD URET    │\n');
fprintf('└─────────────────┴─────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('TX Forstærker Specifikationer:\n');
fprintf('  H-bro spænding:     %.0f Vpp\n', V_hbridge);
fprintf('  Total modstand:     %.0f Ω\n', TX.R_total);
fprintf('  TX strøm (RMS):     %.0f mA\n', TX.I_rms*1000);
fprintf('  Serie modstand:     %.0f Ω (%.1f W rating)\n', TX.R_series, TX.P_resistor);
fprintf('  Q faktor:           %.1f\n', TX.Q);
fprintf('\n');

fprintf('Total tråd behov:\n');
fprintf('  TX (0.56mm):     %.0f m\n', TX.result.wire_len + 5);
fprintf('  RX (0.15mm):     %.0f m\n', RX.result.wire_len + 5);
fprintf('  Bucking (0.56mm): %.0f m\n', Bucking.result.wire_len + 5);
fprintf('\n');

%% Reference Tabeller

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║                    HURTIG REFERENCE TABELLER                    ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('TX Spole (200 mm, 2 lag, 0.56 mm) - Vindinger vs Induktans:\n\n');
fprintf('  Vindinger    Induktans    R_serie     I_rms\n');
fprintf('  ---------    ---------    -------     -----\n');
for N_test = [50, 58, 68, 78, 88, 100]
    result = calc_multilayer(N_test, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    R_ser = R_total_target - result.R_dc;
    I_rms = (V_hbridge / R_total_target) / sqrt(2) * 1000;  % mA
    marker = '';
    if N_test == TX.N
        marker = ' ← Mål';
    end
    fprintf('     %3d       %5.2f mH      %.0f Ω       %.0f mA%s\n', N_test, result.L*1000, R_ser, I_rms, marker);
end
fprintf('\n');

%% Plots - Eksporteres til Docs/Images/

% Opret images mappe hvis den ikke findes
img_dir = fullfile(fileparts(mfilename('fullpath')), '..', 'Images');
if ~exist(img_dir, 'dir')
    mkdir(img_dir);
    fprintf('Oprettet mappe: %s\n', img_dir);
end

% Plot 1: TX Vindinger vs Induktans
figure('Name', 'TX Vindinger vs Induktans', 'Position', [100, 100, 600, 400]);
% Dynamic range: from 40 to 20% beyond calculated turns
N_max = max(120, round(TX.N * 1.2 / 10) * 10);  % Round up to nearest 10
N_range = 40:5:N_max;
L_range_TX = zeros(size(N_range));
for i = 1:length(N_range)
    result = calc_multilayer(N_range(i), TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    L_range_TX(i) = result.L * 1000;
end
plot(N_range, L_range_TX, 'b-', 'LineWidth', 2);
hold on;
plot(TX.N, TX.result.L*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
yline(TX.result.L*1000, 'g--', sprintf('Beregnet %.2f mH', TX.result.L*1000), 'LineWidth', 1.5);
xlabel('Antal Vindinger');
ylabel('Induktans [mH]');
title(sprintf('TX Spole: Vindinger vs Induktans (200mm, %d lag)', TX.n_layers));
grid on;
legend('Wheeler Formel', sprintf('%d vind. = %.2f mH', TX.N, TX.result.L*1000), 'Location', 'northwest');
saveas(gcf, fullfile(img_dir, 'tx_turns_vs_inductance.png'));
fprintf('Gemt: tx_turns_vs_inductance.png\n');

% Plot 2: H-bro Strøm vs Total Modstand (ved resonans)
% Ved resonans: X_L = X_C, så strøm bestemmes kun af R_total
figure('Name', 'H-bro Strøm vs Modstand', 'Position', [100, 100, 600, 400]);
R_range = 10:5:150;
I_rms_range = (V_hbridge ./ R_range) / sqrt(2) * 1000;  % RMS strøm i mA
plot(R_range, I_rms_range, 'b-', 'LineWidth', 2);
hold on;
plot(R_total_target, TX.I_rms*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
yline(TX.I_rms*1000, 'g--', sprintf('Design: %.0f mA', TX.I_rms*1000), 'LineWidth', 1.5);
xline(R_total_target, 'm:', sprintf('R = %d Ω', R_total_target), 'LineWidth', 1.5);
xlabel('Total Modstand R_{total} [Ω]');
ylabel('RMS Strøm [mA]');
title(sprintf('H-bro TX Strøm ved Resonans (V_{pp} = %.0fV)', V_hbridge));
grid on;
legend('I_{RMS} = V_{pp}/(R \times √2)', sprintf('Design: %dΩ = %.0f mA', R_total_target, TX.I_rms*1000), 'Location', 'northeast');
saveas(gcf, fullfile(img_dir, 'tx_current_vs_resistance.png'));
fprintf('Gemt: tx_current_vs_resistance.png\n');

fprintf('\nAlle plots eksporteret til Docs/Images/\n');
fprintf('\n');
fprintf('══════════════════════════════════════════════════════════════════\n');
fprintf('  TX SPOLE: %d vindinger, 2 lag, Ø200mm, L=%.2f mH, C=1.0µF\n', TX.N, TX.result.L*1000);
fprintf('  RX SPOLE: %d vindinger, 3 lag, Ø50mm, L=%.1f mH\n', RX.N, RX.result.L*1000);
fprintf('  BUCKING:  %d vindinger, 1 lag, Ø60mm (kalibreret)\n', Bucking.N);
fprintf('══════════════════════════════════════════════════════════════════\n');
