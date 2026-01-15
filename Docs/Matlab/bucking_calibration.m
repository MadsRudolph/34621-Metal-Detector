%% Bucking Coil Calibration Tool
% Interactive script to find optimal bucking coil turns for TX null
% DTU 34621 VLF Metal Detector Project
%
% Usage: Run this script and follow the prompts to log measurements

clear; clc;

fprintf('\n');
fprintf('══════════════════════════════════════════════════════════════════\n');
fprintf('           BUCKING COIL CALIBRATION TOOL\n');
fprintf('     Find optimal turns for TX field cancellation\n');
fprintf('══════════════════════════════════════════════════════════════════\n\n');

%% Wheeler Formula Function

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
    else
        L = (0.315 * r_cm^2 * N_total^2) / (6*r_cm + 9*l_cm + 10*w_cm) * 1e-6;
    end

    wire_len = 0;
    for layer = 1:n_layers
        r_layer = r_inner + (layer - 0.5) * wire_d;
        wire_len = wire_len + turns_per_layer * 2 * pi * r_layer;
    end

    area = pi * (wire_d/2)^2;
    R_dc = rho * wire_len / area;

    result.L = L;
    result.R_dc = R_dc;
end

%% Persistent Data File

data_file = 'bucking_calibration_data.mat';
rho_copper = 1.68e-8;

%% Load or Initialize Data

if exist(data_file, 'file')
    load(data_file, 'cal_data');
    fprintf('Loaded existing calibration data (%d measurements)\n\n', size(cal_data.log, 1));
else
    cal_data.start_turns = 35;
    cal_data.diameter = 60;
    cal_data.wire_d = 0.52;
    cal_data.layers = 1;
    cal_data.unit = 'mV';
    cal_data.log = [];
    fprintf('No existing data found. Starting new calibration.\n\n');
end

%% Main Menu

while true
    fprintf('─────────────────────────────────────────────────────────────────\n');
    fprintf('  MENU\n');
    fprintf('─────────────────────────────────────────────────────────────────\n');
    fprintf('  1. Add measurement\n');
    fprintf('  2. View all measurements\n');
    fprintf('  3. Analyze and find optimal turns\n');
    fprintf('  4. Edit bucking coil parameters\n');
    fprintf('  5. Clear all data\n');
    fprintf('  6. Show calibration procedure\n');
    fprintf('  0. Exit\n');
    fprintf('─────────────────────────────────────────────────────────────────\n');

    choice = input('Choice: ');

    switch choice
        case 1
            %% Add Measurement
            fprintf('\n');
            fprintf('Current bucking coil: %d turns start, %d mm diameter, %.2f mm wire\n', ...
                cal_data.start_turns, cal_data.diameter, cal_data.wire_d);
            fprintf('\n');

            turns = input('Bucking coil turns: ');
            if isempty(turns)
                fprintf('Cancelled.\n\n');
                continue;
            end

            output = input(sprintf('RX output (%s): ', cal_data.unit));
            if isempty(output)
                fprintf('Cancelled.\n\n');
                continue;
            end

            notes = input('Notes (optional): ', 's');

            % Add to log
            new_entry = [turns, output];
            cal_data.log = [cal_data.log; new_entry];

            % Sort by turns descending
            cal_data.log = sortrows(cal_data.log, -1);

            % Save
            save(data_file, 'cal_data');

            fprintf('\nMeasurement added: %d turns -> %.1f %s\n', turns, output, cal_data.unit);
            fprintf('Total measurements: %d\n\n', size(cal_data.log, 1));

        case 2
            %% View All Measurements
            fprintf('\n');
            if isempty(cal_data.log)
                fprintf('No measurements recorded yet.\n\n');
            else
                fprintf('┌───────────────────────────────────────────────────────┐\n');
                fprintf('│  CALIBRATION LOG                                      │\n');
                fprintf('├───────────────────────────────────────────────────────┤\n');
                fprintf('│  #    Turns    Removed    RX Output (%s)            │\n', cal_data.unit);
                fprintf('├───────────────────────────────────────────────────────┤\n');

                for i = 1:size(cal_data.log, 1)
                    turns = cal_data.log(i, 1);
                    output = cal_data.log(i, 2);
                    removed = cal_data.start_turns - turns;
                    fprintf('│  %2d    %3d       %2d        %8.1f                 │\n', ...
                        i, turns, removed, output);
                end

                fprintf('└───────────────────────────────────────────────────────┘\n\n');
            end

        case 3
            %% Analyze
            fprintf('\n');
            if size(cal_data.log, 1) < 2
                fprintf('Need at least 2 measurements to analyze.\n\n');
                continue;
            end

            turns = cal_data.log(:, 1);
            output = cal_data.log(:, 2);
            removed = cal_data.start_turns - turns;

            % Find minimum
            [min_output, min_idx] = min(output);
            optimal_turns = turns(min_idx);
            optimal_removed = cal_data.start_turns - optimal_turns;

            % Reduction from max
            max_output = max(output);
            if max_output > 0 && min_output > 0
                reduction_db = 20 * log10(min_output / max_output);
            else
                reduction_db = NaN;
            end

            fprintf('┌───────────────────────────────────────────────────────┐\n');
            fprintf('│  ANALYSIS RESULTS                                     │\n');
            fprintf('├───────────────────────────────────────────────────────┤\n');
            fprintf('│  Turns    Removed    Output (%s)    Status          │\n', cal_data.unit);
            fprintf('├───────────────────────────────────────────────────────┤\n');

            for i = 1:length(turns)
                rem = cal_data.start_turns - turns(i);
                if i == min_idx
                    stat = '<-- MINIMUM';
                elseif i > 1 && output(i) < output(i-1)
                    stat = 'decreasing';
                elseif i > 1 && output(i) > output(i-1)
                    stat = 'increasing';
                else
                    stat = '';
                end
                fprintf('│   %3d       %2d       %8.1f       %s\n', ...
                    turns(i), rem, output(i), stat);
            end

            fprintf('└───────────────────────────────────────────────────────┘\n\n');

            fprintf('┌───────────────────────────────────────────────────────┐\n');
            fprintf('│  OPTIMAL CONFIGURATION                                │\n');
            fprintf('├───────────────────────────────────────────────────────┤\n');
            fprintf('│  Starting turns:     %3d                              │\n', cal_data.start_turns);
            fprintf('│  OPTIMAL TURNS:      %3d  (remove %d turns)           │\n', optimal_turns, optimal_removed);
            fprintf('│  Minimum output:     %.1f %s                         │\n', min_output, cal_data.unit);
            if ~isnan(reduction_db)
                fprintf('│  Signal reduction:   %.1f dB                         │\n', reduction_db);
            end
            fprintf('└───────────────────────────────────────────────────────┘\n\n');

            % Calculate theoretical inductance at optimal
            opt_result = calc_coil(optimal_turns, cal_data.layers, ...
                cal_data.diameter/2*1e-3, cal_data.wire_d*1e-3, rho_copper);
            fprintf('Theoretical inductance at %d turns: %.3f mH\n\n', optimal_turns, opt_result.L*1000);

            % Plot
            if size(cal_data.log, 1) >= 3
                figure('Name', 'Bucking Coil Calibration', 'Position', [100, 100, 800, 400]);

                subplot(1, 2, 1);
                plot(turns, output, 'b-o', 'LineWidth', 2, 'MarkerSize', 8, 'MarkerFaceColor', 'b');
                hold on;
                plot(optimal_turns, min_output, 'ro', 'MarkerSize', 14, 'MarkerFaceColor', 'r');
                xlabel('Bucking Coil Turns');
                ylabel(sprintf('RX Output [%s]', cal_data.unit));
                title('RX Output vs Bucking Turns');
                grid on;
                legend('Measurements', sprintf('Optimal: %d turns', optimal_turns), 'Location', 'best');
                set(gca, 'XDir', 'reverse');

                subplot(1, 2, 2);
                plot(removed, output, 'b-o', 'LineWidth', 2, 'MarkerSize', 8, 'MarkerFaceColor', 'b');
                hold on;
                plot(optimal_removed, min_output, 'ro', 'MarkerSize', 14, 'MarkerFaceColor', 'r');
                xlabel('Turns Removed from Start');
                ylabel(sprintf('RX Output [%s]', cal_data.unit));
                title('RX Output vs Turns Removed');
                grid on;
                legend('Measurements', sprintf('Optimal: remove %d', optimal_removed), 'Location', 'best');

                % Save plot
                img_dir = fullfile(fileparts(mfilename('fullpath')), '..', 'Images');
                if exist(img_dir, 'dir')
                    saveas(gcf, fullfile(img_dir, 'bucking_calibration.png'));
                    fprintf('Plot saved to: Docs/Images/bucking_calibration.png\n\n');
                end
            end

        case 4
            %% Edit Parameters
            fprintf('\n');
            fprintf('Current parameters:\n');
            fprintf('  Start turns: %d\n', cal_data.start_turns);
            fprintf('  Diameter: %d mm\n', cal_data.diameter);
            fprintf('  Wire diameter: %.2f mm\n', cal_data.wire_d);
            fprintf('  Layers: %d\n', cal_data.layers);
            fprintf('  Measurement unit: %s\n', cal_data.unit);
            fprintf('\n');

            new_starts = input(sprintf('Start turns [%d]: ', cal_data.start_turns));
            if ~isempty(new_starts), cal_data.start_turns = new_starts; end

            new_diam = input(sprintf('Diameter in mm [%d]: ', cal_data.diameter));
            if ~isempty(new_diam), cal_data.diameter = new_diam; end

            new_wire = input(sprintf('Wire diameter in mm [%.2f]: ', cal_data.wire_d));
            if ~isempty(new_wire), cal_data.wire_d = new_wire; end

            new_layers = input(sprintf('Layers [%d]: ', cal_data.layers));
            if ~isempty(new_layers), cal_data.layers = new_layers; end

            new_unit = input(sprintf('Measurement unit [%s]: ', cal_data.unit), 's');
            if ~isempty(new_unit), cal_data.unit = new_unit; end

            save(data_file, 'cal_data');
            fprintf('\nParameters updated.\n\n');

        case 5
            %% Clear Data
            confirm = input('Are you sure you want to clear all data? (yes/no): ', 's');
            if strcmpi(confirm, 'yes')
                cal_data.log = [];
                save(data_file, 'cal_data');
                fprintf('All measurement data cleared.\n\n');
            else
                fprintf('Cancelled.\n\n');
            end

        case 6
            %% Show Procedure
            fprintf('\n');
            fprintf('┌───────────────────────────────────────────────────────┐\n');
            fprintf('│  BUCKING COIL CALIBRATION PROCEDURE                   │\n');
            fprintf('├───────────────────────────────────────────────────────┤\n');
            fprintf('│                                                       │\n');
            fprintf('│  Setup:                                               │\n');
            fprintf('│  1. Connect oscilloscope to RX coil output            │\n');
            fprintf('│  2. Power TX coil at operating frequency (2 kHz)      │\n');
            fprintf('│  3. Position coils in final detector configuration    │\n');
            fprintf('│                                                       │\n');
            fprintf('│  Calibration:                                         │\n');
            fprintf('│  4. Measure RX output voltage (Vpp or RMS)            │\n');
            fprintf('│  5. Record: turns remaining + voltage                 │\n');
            fprintf('│  6. Remove 1 turn from bucking coil                   │\n');
            fprintf('│  7. Measure again                                     │\n');
            fprintf('│  8. Repeat until voltage starts INCREASING            │\n');
            fprintf('│                                                       │\n');
            fprintf('│  Result:                                              │\n');
            fprintf('│  - Optimal = turn count with MINIMUM voltage          │\n');
            fprintf('│  - This is where bucking field cancels TX leakage     │\n');
            fprintf('│                                                       │\n');
            fprintf('│  Tips:                                                │\n');
            fprintf('│  - Start with more turns than needed                  │\n');
            fprintf('│  - Fine adjustment: half-turn increments near null    │\n');
            fprintf('│  - Temperature affects readings slightly              │\n');
            fprintf('│                                                       │\n');
            fprintf('└───────────────────────────────────────────────────────┘\n\n');

        case 0
            %% Exit
            fprintf('\nCalibration data saved. Goodbye!\n\n');
            break;

        otherwise
            fprintf('Invalid choice.\n\n');
    end
end
