%% Coil Design Functions - Quick Reference
% Helper functions for metal detector coil calculations
%
% YOUR DESIGN: Multi-layer solenoid (wound on cylinder)
%              Use wheeler_multilayer and wheeler_multilayer_turns
%
% Usage: Copy these functions to your MATLAB path or include in your script

%% ========================================================================
%  MULTI-LAYER SOLENOID FORMULAS - USE THESE FOR YOUR DESIGN
%  ========================================================================

function L = wheeler_multilayer(N, r, l, w)
    % Wheeler formula for multi-layer solenoid
    %
    % THIS IS THE CORRECT FORMULA FOR YOUR MULTI-LAYER COIL DESIGN
    %
    % Inputs:
    %   N - Total number of turns
    %   r - Mean radius of coil [m]
    %   l - Length of coil (axial) [m]
    %   w - Radial build (depth of windings) [m]
    % Output:
    %   L - Inductance [H]
    %
    % Formula: L (µH) = (0.8 × r² × N²) / (6r + 9l + 10w)  [r, l, w in cm]
    
    r_cm = r * 100;
    l_cm = l * 100;
    w_cm = w * 100;
    L = (0.8 * r_cm^2 * N^2) / (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
end

function N = wheeler_multilayer_turns(L_target, n_layers, r_inner, wire_d)
    % Calculate turns needed for target inductance (multi-layer)
    % Iterates because geometry depends on number of turns
    %
    % Inputs:
    %   L_target - Target inductance [H]
    %   n_layers - Number of layers
    %   r_inner  - Inner radius of coil (form radius) [m]
    %   wire_d   - Wire diameter [m]
    % Output:
    %   N        - Total number of turns (rounded)
    
    N = 100;  % Initial guess
    for iter = 1:100
        turns_per_layer = N / n_layers;
        l = turns_per_layer * wire_d;  % Axial length
        w = n_layers * wire_d;          % Radial build
        r_mean = r_inner + w/2;         % Mean radius
        
        L = wheeler_multilayer(N, r_mean, l, w);
        
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
%  SINGLE-LAYER SOLENOID FORMULAS - Reference only
%  ========================================================================

function L = wheeler_solenoid(N, r, l)
    % Wheeler formula for single-layer solenoid (wound on cylinder)
    % 
    % Inputs:
    %   N - Number of turns
    %   r - Radius of coil [m]
    %   l - Length of coil (axial extent of windings) [m]
    % Output:
    %   L - Inductance [H]
    
    r_cm = r * 100;
    l_cm = l * 100;
    L = (0.394 * r_cm^2 * N^2) / (9*r_cm + 10*l_cm) * 1e-6;
end

function N = wheeler_solenoid_turns(L_target, r, wire_d)
    % Calculate turns needed for target inductance (single-layer solenoid)
    
    r_cm = r * 100;
    wire_d_cm = wire_d * 100;
    L_target_uH = L_target * 1e6;
    
    N = 100;
    for iter = 1:50
        l_cm = N * wire_d_cm;
        L_uH = (0.394 * r_cm^2 * N^2) / (9*r_cm + 10*l_cm);
        N_new = N * sqrt(L_target_uH / L_uH);
        if abs(N_new - N) < 0.5
            break;
        end
        N = N_new;
    end
    N = round(N);
end

%% ========================================================================
%  WIRE CALCULATIONS
%  ========================================================================

function len = wire_length_solenoid(N, r)
    % Wire length for solenoid coil
    len = N * 2 * pi * r;
end

function R = wire_dc_resistance(length, diameter, rho)
    % DC resistance of wire
    % rho for copper = 1.68e-8 Ohm*m
    area = pi * (diameter/2)^2;
    R = rho * length / area;
end

%% ========================================================================
%  RESONANCE CALCULATIONS
%  ========================================================================

function f = resonant_frequency(L, C)
    % Resonant frequency of LC circuit
    f = 1 / (2*pi*sqrt(L*C));
end

function C = capacitance_for_resonance(L, f)
    % Capacitance needed for target resonant frequency
    C = 1 / ((2*pi*f)^2 * L);
end

%% ========================================================================
%  IMPEDANCE CALCULATIONS
%  ========================================================================

function Z = coil_impedance(R, L, f)
    % Impedance magnitude of coil at frequency
    X_L = 2*pi*f*L;
    Z = sqrt(R^2 + X_L^2);
end

function Q = q_factor(R, L, f)
    % Q factor of coil
    X_L = 2*pi*f*L;
    Q = X_L / R;
end

%% ========================================================================
%  TX COIL - READY TO IMPLEMENT
%  ========================================================================
%
%  Form diameter:    250 mm
%  Wire diameter:    0.52 mm (AWG 24)
%  Total turns:      105
%  Turns per layer:  35
%  Layers:           3
%  Axial length:     18 mm
%  Radial build:     1.6 mm
%  Inductance:       15 mH
%  Wire length:      83 m
%  Winding:          CLOCKWISE
%
%  RESONANCE CAPACITOR:
%  ---------------------
%  Value:            470 nF (film capacitor)
%  Voltage:          >= 25V
%  Connection:       SERIES with TX coil
%  Purpose:          Converts square wave → sine wave
%  f_resonant:       ~1900 Hz
%
%% ========================================================================
%  RX COIL - PLACEHOLDER (TBD)
%  ========================================================================
%
%  Form diameter:    TBD (placeholder: 80 mm)
%  Wire diameter:    TBD (can use AWG 26-30)
%  Target L:         >= 10 mH (requirement 16)
%  Winding:          COUNTER-CLOCKWISE
%
%  Wire options for ~10mH on 80mm form:
%  -------------------------------------
%  AWG 24 (0.52mm): 5 layers, 180 turns, 19mm tall, R=3.7Ω
%  AWG 26 (0.40mm): 4 layers, 177 turns, 18mm tall, R=6.1Ω
%  AWG 28 (0.32mm): 4 layers, 169 turns, 14mm tall, R=9.0Ω
%  AWG 30 (0.25mm): 2 layers, 188 turns, 24mm tall, R=16Ω
%
%  RESONANCE CAPACITOR (TBD):
%  --------------------------
%  Value:            ~680 nF (depends on final L)
%  Connection:       PARALLEL with RX+Bucking
%  Purpose:          Bandpass filter + voltage gain
%  Formula:          C = 1 / (4*pi^2 * f^2 * L)
%
%% ========================================================================
%  BUCKING COIL - PLACEHOLDER (TBD)
%  ========================================================================
%
%  Form diameter:    TBD (between TX and RX)
%  Starting turns:   ~40 (adjust empirically for null)
%  Winding:          COUNTER-CLOCKWISE (same as RX)
%  Connection:       In series with RX
%
%% ========================================================================
%  CIRCUIT CONFIGURATIONS
%  ========================================================================
%
%  TX: SERIES LC (square wave → sine wave)
%  ----------------------------------------
%       MCU → Driver → [C 470nF] → [L 15mH] → GND
%
%  RX: PARALLEL LC (bandpass + gain)
%  ---------------------------------
%          ┌──[RX+Buck ~11mH]──┐
%       ───┤                   ├─── To Preamp
%          └──[C ~680nF]───────┘
%
