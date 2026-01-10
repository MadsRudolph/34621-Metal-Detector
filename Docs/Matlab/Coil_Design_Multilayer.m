%% Multi-Layer Solenoid Coil Design Calculator
% DTU 34621 VLF Metaldetektor - Arduino Nano Design
% Se Docs/Theory/Coil Design.md for fuld dokumentation

clear; clc; close all;

%% Parametre - Arduino Nano Design

% Operating parameters
f_tx = 2000;                    % TX frequency [Hz]
V_supply = 7.5;                 % Design voltage [V] (mid-life battery)
I_target = 80e-3;               % Target TX current [A] (Arduino Nano budget)

% TX Spole - matcher H-bro forstærker (L = 6.33 mH)
TX.wire_diameter = 0.52e-3;     % 0.52 mm = AWG 24
TX.wire_rho = 1.68e-8;          % Kobber resistivitet [Ohm*m]
TX.diameter = 200e-3;           % 200 mm form
TX.L_target = 6.33e-3;          % 6.33 mH (matcher forstærker)
TX.n_layers = 2;

% RX Spole - krav: L >= 10 mH
RX.wire_diameter = 0.32e-3;     % 0.32 mm = AWG 28
RX.wire_rho = 1.68e-8;
RX.diameter = 80e-3;            % 80 mm form
RX.L_target = 12e-3;            % 12 mH (> 10 mH krav)
RX.n_layers = 4;

% Bucking Spole - justeres empirisk
Bucking.wire_diameter = 0.52e-3;
Bucking.wire_rho = 1.68e-8;
Bucking.diameter = 120e-3;      % 120 mm form
Bucking.n_layers = 1;
Bucking.N_start = 35;           % Start værdi (juster for null)

%% Hjælpefunktioner

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

%% TX Spole Beregning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       TX SPOLE - OPDATERET TIL ARDUINO NANO                     ║\n');
fprintf('║       Matcher H-bro forstærker med L = 6.33 mH                  ║\n');
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

% Resonance capacitor for 2 kHz
TX.C_resonance = 1 / (4*pi^2*f_tx^2*TX.result.L);
TX.C_std = 1.0e-6;  % Standard value: 1.0 µF
TX.f_resonant = 1 / (2*pi*sqrt(TX.result.L * TX.C_std));

fprintf('Design Parametre:\n');
fprintf('  Form diameter:      %.0f mm\n', TX.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm (AWG 24)\n', TX.wire_diameter*1000);
fprintf('  Mål induktans:      %.2f mH (matcher forstærker)\n', TX.L_target*1000);
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
fprintf('  DC Modstand:        %.2f Ω\n', TX.result.R_dc);
fprintf('\n');

fprintf('Elektriske @ %d Hz, %.1f V:\n', f_tx, V_supply);
fprintf('  Reaktans (X_L):     %.1f Ω\n', TX.X_L);
fprintf('  Impedans (Z):       %.1f Ω\n', TX.Z);
fprintf('  Strøm:              %.1f mA (mål: %.0f mA)\n', TX.I*1000, I_target*1000);
fprintf('  Q faktor:           %.1f\n', TX.Q);
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

% Warning about voltage rating
V_peak_resonance = TX.Q * V_supply;
fprintf('  ⚠ ADVARSEL: Ved resonans kan spænding over C nå %.0f V!\n', V_peak_resonance);
fprintf('              Brug kondensator med rating >= 50V\n');
fprintf('\n');

%% RX Spole Beregning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║       RX SPOLE - Krav: L >= 10 mH                               ║\n');
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

fprintf('Design Parametre:\n');
fprintf('  Form diameter:      %.0f mm\n', RX.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm (AWG 28)\n', RX.wire_diameter*1000);
fprintf('  Mål induktans:      %.1f mH (krav: >= 10 mH)\n', RX.L_target*1000);
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
fprintf('║     BUCKING SPOLE - Justeres empirisk                           ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

Bucking.r_inner = Bucking.diameter / 2;

% Calculate parameters for starting turns
Bucking.result = calc_multilayer(Bucking.N_start, Bucking.n_layers, Bucking.r_inner, Bucking.wire_diameter, Bucking.wire_rho);

fprintf('Start Parametre (juster efter behov):\n');
fprintf('  Form diameter:      %.0f mm (mellem TX og RX)\n', Bucking.diameter*1000);
fprintf('  Tråddiameter:       %.2f mm\n', Bucking.wire_diameter*1000);
fprintf('  Start vindinger:    %d (JUSTER EMPIRISK for null)\n', Bucking.N_start);
fprintf('  Antal lag:          %d\n', Bucking.n_layers);
fprintf('  Vikleretning:       MOD URET (samme som RX)\n');
fprintf('\n');

fprintf('┌────────────────────────────────────────────────────────────────┐\n');
fprintf('│  BUCKING SPOLE VIKLINGS SPECIFIKATION                         │\n');
fprintf('├────────────────────────────────────────────────────────────────┤\n');
fprintf('│  Start vindinger:  %4d vindinger (juster 25-50 for null)     │\n', Bucking.N_start);
fprintf('│  Vindinger per lag:%4.0f vindinger                            │\n', Bucking.result.turns_per_layer);
fprintf('│  Antal lag:        %4d lag                                   │\n', Bucking.n_layers);
fprintf('│  Aksial længde:    %5.1f mm                                  │\n', Bucking.result.l*1000);
fprintf('│  Vikleretning:     MOD URET                                  │\n');
fprintf('└────────────────────────────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Beregnede Resultater:\n');
fprintf('  Induktans:          %.2f mH\n', Bucking.result.L*1000);
fprintf('  Trådlængde:         %.1f m\n', Bucking.result.wire_len);
fprintf('  DC Modstand:        %.2f Ω\n', Bucking.result.R_dc);
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
fprintf('│ Antal vindinger │    %2d vind. │   %3d vind. │    %2d vind. │\n', TX.N, RX.N, Bucking.N_start);
fprintf('│ Antal lag       │     %d       │     %d       │     %d       │\n', TX.n_layers, RX.n_layers, Bucking.n_layers);
fprintf('│ Aksial længde   │   %4.1f mm   │   %4.1f mm   │   %4.1f mm   │\n', TX.result.l*1000, RX.result.l*1000, Bucking.result.l*1000);
fprintf('│ Induktans       │   %4.2f mH  │   %4.1f mH   │   %4.2f mH  │\n', TX.result.L*1000, RX.result.L*1000, Bucking.result.L*1000);
fprintf('│ DC Modstand     │   %4.2f Ω    │   %4.1f Ω    │   %4.2f Ω    │\n', TX.result.R_dc, RX.result.R_dc, Bucking.result.R_dc);
fprintf('│ Trådlængde      │   %4.1f m    │   %4.1f m    │   %4.1f m    │\n', TX.result.wire_len, RX.result.wire_len, Bucking.result.wire_len);
fprintf('│ Resonans C      │   1.0 µF    │   470 nF    │    N/A      │\n');
fprintf('├─────────────────┼─────────────┴─────────────┴─────────────┤\n');
fprintf('│ Vikleretning    │  MED URET     MOD URET      MOD URET    │\n');
fprintf('└─────────────────┴─────────────────────────────────────────┘\n');
fprintf('\n');

fprintf('Total tråd behov:\n');
fprintf('  TX (0.52mm):     %.0f m\n', TX.result.wire_len + 5);
fprintf('  RX (0.32mm):     %.0f m\n', RX.result.wire_len + 5);
fprintf('  Bucking (0.52mm): %.0f m\n', Bucking.result.wire_len + 5);
fprintf('\n');

%% Nano vs Mega Sammenligning

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║              ARDUINO NANO vs MEGA SAMMENLIGNING                 ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('┌─────────────────────┬──────────────┬──────────────┬────────────┐\n');
fprintf('│ Parameter           │ Arduino Mega │ Arduino Nano │ Forbedring │\n');
fprintf('├─────────────────────┼──────────────┼──────────────┼────────────┤\n');
fprintf('│ Elektronik strøm    │    80 mA     │    40 mA     │   -50%%     │\n');
fprintf('│ Tilgængelig TX strøm│    40 mA     │    80 mA     │  +100%%     │\n');
fprintf('│ TX induktans        │    15 mH     │   6.3 mH     │   -58%%     │\n');
fprintf('│ TX impedans @ 2kHz  │   189 Ω      │    80 Ω      │   -58%%     │\n');
fprintf('│ Magnetisk felt      │   Baseline   │   ~200%%      │  +100%%     │\n');
fprintf('│ Detektionsdybde     │   Baseline   │   +26%%       │   +26%%     │\n');
fprintf('└─────────────────────┴──────────────┴──────────────┴────────────┘\n');
fprintf('\n');
fprintf('  → Arduino Nano giver DOBBELT magnetfelt = +26%% dybere detektion!\n');
fprintf('\n');

%% Reference Tabeller

fprintf('\n');
fprintf('╔══════════════════════════════════════════════════════════════════╗\n');
fprintf('║                    HURTIG REFERENCE TABELLER                    ║\n');
fprintf('╚══════════════════════════════════════════════════════════════════╝\n\n');

fprintf('TX Spole (200 mm, 2 lag, AWG 24) - Vindinger vs Induktans:\n\n');
fprintf('  Vindinger    Induktans    Strøm @ 7.5V\n');
fprintf('  ---------    ---------    ------------\n');
for N_test = [50, 58, 68, 78, 88, 100]
    result = calc_multilayer(N_test, TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    X_L = 2*pi*f_tx*result.L;
    Z = sqrt(result.R_dc^2 + X_L^2);
    I = V_supply / Z * 1000;
    marker = '';
    if N_test == TX.N
        marker = ' ← Mål';
    end
    fprintf('     %3d       %5.2f mH      %5.1f mA%s\n', N_test, result.L*1000, I, marker);
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
N_range = 40:5:120;
L_range_TX = zeros(size(N_range));
for i = 1:length(N_range)
    result = calc_multilayer(N_range(i), TX.n_layers, TX.r_inner, TX.wire_diameter, TX.wire_rho);
    L_range_TX(i) = result.L * 1000;
end
plot(N_range, L_range_TX, 'b-', 'LineWidth', 2);
hold on;
plot(TX.N, TX.result.L*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
yline(TX.L_target*1000, 'g--', sprintf('Mål %.1f mH', TX.L_target*1000), 'LineWidth', 1.5);
xlabel('Antal Vindinger');
ylabel('Induktans [mH]');
title(sprintf('TX Spole: Vindinger vs Induktans (200mm, %d lag)', TX.n_layers));
grid on;
legend('Wheeler Formel', sprintf('%d vind. = %.2f mH', TX.N, TX.result.L*1000), 'Location', 'northwest');
saveas(gcf, fullfile(img_dir, 'tx_turns_vs_inductance.png'));
fprintf('Gemt: tx_turns_vs_inductance.png\n');

% Plot 2: TX Strøm vs Induktans
figure('Name', 'TX Strøm vs Induktans', 'Position', [100, 100, 600, 400]);
L_range = 3:0.5:15;
I_range = zeros(size(L_range));
for i = 1:length(L_range)
    X_L = 2*pi*f_tx*L_range(i)/1000;
    R_dc_est = TX.result.R_dc * (L_range(i)/1000 / TX.result.L)^0.5;
    Z = sqrt(R_dc_est^2 + X_L^2);
    I_range(i) = V_supply / Z * 1000;
end
plot(L_range, I_range, 'b-', 'LineWidth', 2);
hold on;
plot(TX.result.L*1000, TX.I*1000, 'ro', 'MarkerSize', 12, 'MarkerFaceColor', 'r');
yline(I_target*1000, 'g--', sprintf('Mål %.0f mA', I_target*1000), 'LineWidth', 1.5);
xlabel('Induktans [mH]');
ylabel('Strøm [mA]');
title(sprintf('TX Strøm vs Induktans @ %.1fV, %d Hz', V_supply, f_tx));
grid on;
legend('Strøm vs L', sprintf('%.2f mH = %.1f mA', TX.result.L*1000, TX.I*1000), 'Location', 'northeast');
saveas(gcf, fullfile(img_dir, 'tx_current_vs_inductance.png'));
fprintf('Gemt: tx_current_vs_inductance.png\n');

% Plot 3: Nano vs Mega Sammenligning
figure('Name', 'Nano vs Mega', 'Position', [100, 100, 500, 400]);
categories = {'Elektronik [mA]', 'TX Strøm [mA]', 'Magnetfelt [%]'};
mega_values = [80, 40, 100];
nano_values = [40, 80, 200];
x = 1:3;
bar(x, [mega_values; nano_values]', 'grouped');
set(gca, 'XTickLabel', categories);
ylabel('Værdi');
title('Arduino Nano vs Mega Sammenligning');
legend('Arduino Mega', 'Arduino Nano', 'Location', 'northwest');
grid on;
saveas(gcf, fullfile(img_dir, 'nano_vs_mega.png'));
fprintf('Gemt: nano_vs_mega.png\n');

fprintf('\nAlle plots eksporteret til Docs/Images/\n');
fprintf('\n');
fprintf('══════════════════════════════════════════════════════════════════\n');
fprintf('  TX SPOLE: %d vindinger, 2 lag, Ø200mm, L=%.2f mH, C=1.0µF\n', TX.N, TX.result.L*1000);
fprintf('  RX SPOLE: %d vindinger, 4 lag, Ø80mm, L=%.1f mH\n', RX.N, RX.result.L*1000);
fprintf('  BUCKING:  ~%d vindinger, 1 lag, Ø120mm (juster for null)\n', Bucking.N_start);
fprintf('══════════════════════════════════════════════════════════════════\n');
