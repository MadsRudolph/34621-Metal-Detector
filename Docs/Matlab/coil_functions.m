%% Coil Design Functions
% Hjælpefunktioner til spoleberegning - DTU 34621 VLF Metaldetektor
% Se Docs/Theory/Coil Design.md for fuld dokumentation

%% Multi-layer solenoid (Wheeler formel)

function L = wheeler_multilayer(N, r, l, w)
    % L = induktans [H], N = vindinger, r = middelradius [m]
    % l = aksial længde [m], w = radial tykkelse [m]
    r_cm = r * 100; l_cm = l * 100; w_cm = w * 100;
    L = (0.8 * r_cm^2 * N^2) / (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
end

function N = wheeler_multilayer_turns(L_target, n_layers, r_inner, wire_d)
    % Beregn vindinger for mål-induktans (iterativ)
    N = 100;
    for iter = 1:100
        turns_per_layer = N / n_layers;
        l = turns_per_layer * wire_d;
        w = n_layers * wire_d;
        r_mean = r_inner + w/2;
        L = wheeler_multilayer(N, r_mean, l, w);
        if L <= 0, break; end
        N_new = N * sqrt(L_target / L);
        if abs(N_new - N) < 0.5, break; end
        N = N_new;
    end
    N = round(N);
end

%% Single-layer solenoid (reference)

function L = wheeler_solenoid(N, r, l)
    r_cm = r * 100; l_cm = l * 100;
    L = (0.394 * r_cm^2 * N^2) / (9*r_cm + 10*l_cm) * 1e-6;
end

function N = wheeler_solenoid_turns(L_target, r, wire_d)
    r_cm = r * 100; wire_d_cm = wire_d * 100;
    L_target_uH = L_target * 1e6;
    N = 100;
    for iter = 1:50
        l_cm = N * wire_d_cm;
        L_uH = (0.394 * r_cm^2 * N^2) / (9*r_cm + 10*l_cm);
        N_new = N * sqrt(L_target_uH / L_uH);
        if abs(N_new - N) < 0.5, break; end
        N = N_new;
    end
    N = round(N);
end

%% Wire og impedans

function len = wire_length_solenoid(N, r)
    len = N * 2 * pi * r;
end

function R = wire_dc_resistance(length, diameter, rho)
    % rho for kobber = 1.68e-8 Ohm*m
    area = pi * (diameter/2)^2;
    R = rho * length / area;
end

function f = resonant_frequency(L, C)
    f = 1 / (2*pi*sqrt(L*C));
end

function C = capacitance_for_resonance(L, f)
    C = 1 / ((2*pi*f)^2 * L);
end

function Z = coil_impedance(R, L, f)
    X_L = 2*pi*f*L;
    Z = sqrt(R^2 + X_L^2);
end

function Q = q_factor(R, L, f)
    X_L = 2*pi*f*L;
    Q = X_L / R;
end
