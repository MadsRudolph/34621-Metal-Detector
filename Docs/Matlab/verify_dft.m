%% DFT Verification Script
% Verifies Arduino DFT implementation against MATLAB
%
% USAGE:
%   1. Connect Arduino with CAPTURE_SAMPLES firmware
%   2. Run: verify_dft()
%   3. Select your COM port
%   4. See results and graphs
%
% DTU 34621 - VLF Metal Detector

function verify_dft()

    %% ==================== CONFIGURATION ====================
    BAUD_RATE = 115200;
    N = 64;                     % Samples per DFT window
    F_SAMPLE = 8000;            % Sample rate (Hz)
    F_SIGNAL = 2000;            % Signal frequency (Hz)

    fprintf('\n========================================\n');
    fprintf('   DFT Verification - VLF Metal Detector\n');
    fprintf('========================================\n\n');

    %% ==================== CONNECT TO ARDUINO ====================

    % Show available ports and let user choose
    ports = serialportlist("available");
    if isempty(ports)
        error('No serial ports found. Connect Arduino and try again.');
    end

    fprintf('Available ports:\n');
    for i = 1:length(ports)
        fprintf('  [%d] %s\n', i, ports(i));
    end
    choice = input('\nSelect port number: ');
    port = char(ports(choice));

    fprintf('\nConnecting to %s...\n', port);
    s = serialport(port, BAUD_RATE);
    s.Timeout = 5;

    % Wait for Arduino to be ready (it resets when serial connects)
    fprintf('Waiting for Arduino READY signal...\n');
    pause(2);  % Give Arduino time to reset
    flush(s);  % Clear any garbage from reset

    % Wait for READY signal
    ready_data = '';
    for attempt = 1:30  % Max 3 seconds
        pause(0.1);
        if s.NumBytesAvailable > 0
            ready_data = [ready_data, char(read(s, s.NumBytesAvailable, 'uint8'))]; %#ok<AGROW>
        end
        if contains(ready_data, 'READY')
            fprintf('Arduino ready!\n');
            break;
        end
    end

    %% ==================== TRIGGER CAPTURE ====================

    fprintf('Triggering capture...\n');
    fwrite(s, 'C');  % Use fwrite - write() blocks

    % Read until we get "End Capture"
    raw_data = '';
    for attempt = 1:50  % Max 5 seconds
        pause(0.1);
        if s.NumBytesAvailable > 0
            raw_data = [raw_data, char(read(s, s.NumBytesAvailable, 'uint8'))]; %#ok<AGROW>
        end
        if contains(raw_data, 'End Capture')
            fprintf('Capture complete!\n\n');
            break;
        end
    end

    if ~contains(raw_data, 'End Capture')
        fprintf('Warning: Capture may have timed out.\n');
        fprintf('Raw data received: %s\n', raw_data);
    end

    delete(s);  % Close serial connection
    clear s;

    %% ==================== PARSE DATA ====================

    % Initialize
    samples = zeros(N, 1);
    sample_count = 0;
    arduino.Re = 0;
    arduino.Im = 0;
    arduino.Mag = 0;
    arduino.Phase = 0;

    % Parse line by line
    lines = strsplit(raw_data, {'\r\n', '\n', '\r'});
    section = 'none';

    for i = 1:length(lines)
        line = strtrim(lines{i});

        % Track which section we're in
        if contains(line, 'sample,value'), section = 'samples'; continue; end
        if contains(line, 'Arduino Results'), section = 'results'; continue; end
        if contains(line, 'End Capture'), break; end

        % Skip headers and empty lines
        if isempty(line) || startsWith(line, '---') || strcmp(line, 'ACK')
            continue;
        end

        % Parse based on section
        if strcmp(section, 'samples') && contains(line, ',')
            parts = strsplit(line, ',');
            if length(parts) == 2
                sample_count = sample_count + 1;
                samples(sample_count) = str2double(parts{2});
            end
        end

        if strcmp(section, 'results') && contains(line, ',')
            parts = strsplit(line, ',');
            if length(parts) == 2
                name = strtrim(parts{1});
                value = str2double(parts{2});
                switch name
                    case 'Re',    arduino.Re = value;
                    case 'Im',    arduino.Im = value;
                    case 'Mag',   arduino.Mag = value;
                    case 'Phase', arduino.Phase = value;
                end
            end
        end
    end

    fprintf('Parsed %d samples\n\n', sample_count);

    if sample_count ~= N
        error('Expected %d samples, got %d. Check Arduino connection.', N, sample_count);
    end

    %% ==================== CALCULATE DFT (DSP Course Convention) ====================
    %
    % We use MATLAB's FFT to verify Arduino's optimized algorithm.
    % This is a DIFFERENT method - if results match, Arduino is correct!
    %
    % Arduino: Optimized 4-sample accumulation (only computes bin k=16)
    % MATLAB:  Full FFT using Cooley-Tukey algorithm (computes all bins)
    %
    % DSP Course Convention (62743):
    %   X = fftshift(fft(x)) / N      (normalized, DC-centered)
    %   f = (-N/2:N/2-1) * (Fs/N)     (symmetric frequency axis)

    % Compute normalized, DC-centered FFT (course convention)
    X = fftshift(fft(samples)) / N;
    f_axis = (-N/2:N/2-1)' * (F_SAMPLE/N);  % Frequency axis [Hz]

    % Find index of target frequency (2kHz)
    [~, idx] = min(abs(f_axis - F_SIGNAL));
    X_2kHz = X(idx);

    % The DFT formula is: X[k] = (1/N) * sum( x[n] * e^(-j*2*pi*k*n/N) )
    % Which expands to: X[k] = (1/N) * sum( x[n] * (cos(2*pi*k*n/N) - j*sin(2*pi*k*n/N)) )
    %
    % Arduino calculates (unnormalized):
    %     Re = sum( x[n] * cos(...) )
    %     Im = sum( x[n] * (-sin(...)) )
    %
    % So we scale back by N for comparison:
    Re = real(X_2kHz) * N;
    Im = imag(X_2kHz) * N;

    % Magnitude and phase (same formula as Arduino)
    Mag = floor(sqrt(Re^2 + Im^2) / N);
    Phase = round(atan2(Im, Re) * 180 / pi);

    fprintf('Method: FFT with fftshift/N (DSP course convention)\n');
    fprintf('Extracted bin at f = %.0f Hz (index %d)\n\n', f_axis(idx), idx);

    %% ==================== COMPARE RESULTS ====================

    fprintf('=== VERIFICATION ===\n\n');
    fprintf('Arduino: Optimized 4-sample DFT (only bin k=16)\n');
    fprintf('MATLAB:  X = fftshift(fft(x))/N, extract f=2kHz\n\n');
    fprintf('If results match, Arduino algorithm is mathematically correct!\n\n');

    fprintf('         Arduino    MATLAB     Error\n');
    fprintf('Re:      %7.0f   %7.0f   %6.0f\n', arduino.Re, Re, arduino.Re - Re);
    fprintf('Im:      %7.0f   %7.0f   %6.0f\n', arduino.Im, Im, arduino.Im - Im);
    fprintf('Mag:     %7d   %7d   %6d\n', arduino.Mag, Mag, arduino.Mag - Mag);
    fprintf('Phase:   %6d°   %6d°  %6d°\n\n', arduino.Phase, Phase, arduino.Phase - Phase);

    % Small errors expected due to floating point vs integer math
    mag_ok = abs(arduino.Mag - Mag) <= 5;
    phase_ok = abs(arduino.Phase - Phase) <= 3;
    pass = mag_ok && phase_ok;

    if pass
        fprintf('*** PASS - Arduino optimization matches standard FFT! ***\n\n');
    else
        fprintf('*** FAIL - Results do not match ***\n\n');
    end

    %% ==================== PLOT AND SAVE GRAPHS ====================

    IMAGES_DIR = fullfile(fileparts(mfilename('fullpath')), '..', 'Images');
    t = (0:N-1) / F_SAMPLE * 1000;  % Time in ms

    % Normalize phasor values for plots
    arduino_re = arduino.Re / N;
    arduino_im = arduino.Im / N;
    matlab_re = Re / N;
    matlab_im = Im / N;
    max_val = max(abs([arduino_re, arduino_im, matlab_re, matlab_im])) * 1.3;

    % FFT spectrum data (course convention: already normalized and centered)
    % X and f_axis already computed above with fftshift/N convention
    mag_spectrum = abs(X);
    target_idx = idx;  % Index of 2kHz in centered spectrum

    % --- Save Plot 1: Time Domain ---
    fig1 = figure('Name', 'Samples', 'Position', [100 100 700 400]);
    stem(t, samples, 'filled', 'MarkerSize', 4);
    hold on;
    plot(t, samples, 'b-', 'LineWidth', 0.5);
    xlabel('Time (ms)');
    ylabel('ADC Value (DC removed)');
    title(sprintf('Captured Samples (N=%d, Fs=%d Hz)', N, F_SAMPLE));
    grid on;
    xlim([0 max(t)]);
    saveas(fig1, fullfile(IMAGES_DIR, 'dft_samples.png'));

    % --- Save Plot 2: Frequency Spectrum (DSP course style: centered) ---
    fig2 = figure('Name', 'Spectrum', 'Position', [100 100 700 400]);
    stem(f_axis, mag_spectrum, 'filled', 'MarkerSize', 4);
    hold on;
    % Highlight +2kHz peak
    stem(f_axis(target_idx), mag_spectrum(target_idx), 'r', 'filled', 'MarkerSize', 8, 'LineWidth', 2);
    text(f_axis(target_idx) + 100, mag_spectrum(target_idx), sprintf('f = +%d Hz\n|X| = %.1f', F_SIGNAL, mag_spectrum(target_idx)));
    % Highlight -2kHz peak (conjugate symmetry for real signals)
    [~, neg_idx] = min(abs(f_axis + F_SIGNAL));
    stem(f_axis(neg_idx), mag_spectrum(neg_idx), 'r', 'filled', 'MarkerSize', 8, 'LineWidth', 2);
    text(f_axis(neg_idx) - 500, mag_spectrum(neg_idx), sprintf('f = %d Hz\n(conjugate)', -F_SIGNAL), 'HorizontalAlignment', 'right');
    xlabel('Frequency (Hz)');
    ylabel('|X(f)| / N');
    title('FFT Magnitude Spectrum (fftshift, normalized)');
    grid on;
    xlim([-F_SAMPLE/2 F_SAMPLE/2]);
    saveas(fig2, fullfile(IMAGES_DIR, 'dft_spectrum.png'));

    % --- Save Plot 3: Phasor Diagram ---
    fig3 = figure('Name', 'Phasor', 'Position', [100 100 500 500]);
    quiver(0, 0, arduino_re, arduino_im, 0, 'b', 'LineWidth', 2, 'MaxHeadSize', 0.5);
    hold on;
    quiver(0, 0, matlab_re, matlab_im, 0, 'r--', 'LineWidth', 2, 'MaxHeadSize', 0.5);
    xlim([-max_val max_val]);
    ylim([-max_val max_val]);
    axis equal;
    grid on;
    xlabel('Real (Re/N)');
    ylabel('Imaginary (Im/N)');
    title('Phasor Diagram');
    legend('Arduino (optimized)', 'MATLAB FFT', 'Location', 'best');
    saveas(fig3, fullfile(IMAGES_DIR, 'dft_phasor.png'));

    % --- Save Plot 4: Comparison Summary ---
    fig4 = figure('Name', 'Comparison', 'Position', [100 100 600 500]);
    axis off;

    if pass
        result_str = 'PASS';
        result_color = [0 0.7 0];
    else
        result_str = 'FAIL';
        result_color = [0.8 0 0];
    end

    summary = {
        sprintf('Source: Serial %s', port)
        ''
        '--- Verification Method ---'
        'Arduino: Optimized 4-sample DFT (bin k=16)'
        'MATLAB:  X = fftshift(fft(x))/N'
        ''
        '--- Results Comparison ---'
        ''
        '         Arduino    MATLAB     Error'
        sprintf('Re:      %7.0f   %7.0f   %6.0f', arduino.Re, Re, arduino.Re - Re)
        sprintf('Im:      %7.0f   %7.0f   %6.0f', arduino.Im, Im, arduino.Im - Im)
        sprintf('Mag:     %7d   %7d   %6d', arduino.Mag, Mag, arduino.Mag - Mag)
        sprintf('Phase:   %7d   %7d   %6d', arduino.Phase, Phase, arduino.Phase - Phase)
        ''
        '--- Signal Stats ---'
        sprintf('Min: %d  Max: %d  RMS: %.1f', min(samples), max(samples), rms(samples))
    };

    text(0.1, 0.95, summary, 'FontName', 'FixedWidth', 'FontSize', 11, ...
         'VerticalAlignment', 'top', 'Units', 'normalized');
    text(0.5, 0.08, result_str, 'FontSize', 28, 'FontWeight', 'bold', ...
         'Color', result_color, 'HorizontalAlignment', 'center', 'Units', 'normalized');
    title('DFT Verification (DSP Course Convention)');
    saveas(fig4, fullfile(IMAGES_DIR, 'dft_comparison.png'));

    fprintf('Plots saved to Docs/Images/:\n');
    fprintf('  - dft_samples.png\n');
    fprintf('  - dft_spectrum.png\n');
    fprintf('  - dft_phasor.png\n');
    fprintf('  - dft_comparison.png\n\n');

    %% ==================== FINAL OUTPUT ====================

    fprintf('========================================\n');
    if pass
        fprintf('  DFT Verification: PASS\n');
        fprintf('  (fftshift/N matches Arduino)\n');
    else
        fprintf('  DFT Verification: MISMATCH\n');
    end
    fprintf('========================================\n\n');

end
