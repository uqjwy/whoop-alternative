#!/usr/bin/env python3
"""
PPG Signal Analysis Tool
Analyzes PPG simulator output and creates visualizations
"""

import numpy as np
import matplotlib.pyplot as plt
import subprocess
import tempfile
import os

def generate_ppg_data(hr=70, noise=0.1, motion=0.0, sleep=False, duration=30):
    """Generate PPG data using the C simulator"""
    
    # Create temporary C test file
    test_code = f"""
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppg_simulator_host.h"

int main() {{
    struct ppg_sim_config config = {{
        .heart_rate_bpm = {hr},
        .noise_level = {noise},
        .motion_artifacts = {motion},
        .sleep_mode = {1 if sleep else 0},
        .breathing_rate_bpm = 16.0f,
        .signal_quality = 95
    }};
    
    ppg_sim_init(&config);
    
    const int sample_rate = 50;
    const int total_samples = {duration} * sample_rate;
    
    for (int i = 0; i < total_samples; i++) {{
        uint32_t timestamp_ms = i * (1000 / sample_rate);
        float sample = ppg_sim_generate_sample(timestamp_ms);
        printf("%.6f\\n", sample);
    }}
    
    return 0;
}}
"""
    
    # Write, compile and run
    with tempfile.NamedTemporaryFile(mode='w', suffix='.c', delete=False) as f:
        f.write(test_code)
        temp_c = f.name
    
    temp_exe = temp_c.replace('.c', '')
    
    try:
        # Compile
        compile_cmd = f"gcc -Itests {temp_c} -lm -o {temp_exe}"
        subprocess.run(compile_cmd, shell=True, check=True, capture_output=True)
        
        # Run and capture output
        result = subprocess.run(temp_exe, capture_output=True, text=True, check=True)
        
        # Parse data
        data = np.array([float(line.strip()) for line in result.stdout.strip().split('\n') if line.strip()])
        
        return data
        
    finally:
        # Cleanup
        for temp_file in [temp_c, temp_exe]:
            if os.path.exists(temp_file):
                os.unlink(temp_file)

def analyze_ppg_signal(data, sample_rate=50):
    """Analyze PPG signal characteristics"""
    
    time = np.arange(len(data)) / sample_rate
    
    # Basic statistics
    stats = {
        'mean': np.mean(data),
        'std': np.std(data),
        'min': np.min(data),
        'max': np.max(data),
        'range': np.max(data) - np.min(data)
    }
    
    # Heart rate estimation using peak detection
    from scipy.signal import find_peaks
    peaks, _ = find_peaks(data, height=stats['mean'] + 0.5 * stats['std'], distance=sample_rate//3)
    
    if len(peaks) > 1:
        # Calculate time between peaks
        peak_intervals = np.diff(peaks) / sample_rate  # seconds
        mean_interval = np.mean(peak_intervals)
        estimated_hr = 60.0 / mean_interval if mean_interval > 0 else 0
    else:
        estimated_hr = 0
    
    stats['peaks'] = len(peaks)
    stats['estimated_hr'] = estimated_hr
    
    return stats, time, peaks

def plot_ppg_analysis(data, time, peaks, stats, title="PPG Signal Analysis"):
    """Create comprehensive PPG analysis plots"""
    
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
    
    # 1. Time domain signal
    ax1.plot(time, data, 'b-', linewidth=1, label='PPG Signal')
    ax1.plot(time[peaks], data[peaks], 'ro', markersize=4, label=f'Peaks ({len(peaks)})')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('PPG Amplitude')
    ax1.set_title('PPG Signal with Peak Detection')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Frequency domain analysis
    fft = np.fft.fft(data)
    freqs = np.fft.fftfreq(len(data), 1/50)  # 50 Hz sample rate
    magnitude = np.abs(fft)
    
    # Only plot positive frequencies up to 5 Hz
    pos_mask = (freqs >= 0) & (freqs <= 5)
    ax2.plot(freqs[pos_mask], magnitude[pos_mask], 'g-', linewidth=1)
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('Magnitude')
    ax2.set_title('Frequency Spectrum')
    ax2.grid(True, alpha=0.3)
    
    # 3. Signal statistics
    ax3.text(0.1, 0.9, f"Mean: {stats['mean']:.4f}", transform=ax3.transAxes, fontsize=12)
    ax3.text(0.1, 0.8, f"Std Dev: {stats['std']:.4f}", transform=ax3.transAxes, fontsize=12)
    ax3.text(0.1, 0.7, f"Range: {stats['range']:.4f}", transform=ax3.transAxes, fontsize=12)
    ax3.text(0.1, 0.6, f"Peaks: {stats['peaks']}", transform=ax3.transAxes, fontsize=12)
    ax3.text(0.1, 0.5, f"Est. HR: {stats['estimated_hr']:.1f} bpm", transform=ax3.transAxes, fontsize=12)
    ax3.set_title('Signal Statistics')
    ax3.axis('off')
    
    # 4. Histogram
    ax4.hist(data, bins=50, alpha=0.7, color='skyblue', edgecolor='black')
    ax4.axvline(stats['mean'], color='red', linestyle='--', label=f"Mean: {stats['mean']:.3f}")
    ax4.set_xlabel('PPG Amplitude')
    ax4.set_ylabel('Frequency')
    ax4.set_title('Amplitude Distribution')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    
    plt.suptitle(title, fontsize=16)
    plt.tight_layout()
    return fig

def main():
    """Main analysis function"""
    
    print("🔬 PPG Signal Analysis Tool")
    print("===========================")
    
    # Test scenarios
    scenarios = [
        {"name": "Rest - Clean", "hr": 70, "noise": 0.05, "motion": 0.0, "sleep": False},
        {"name": "Exercise", "hr": 140, "noise": 0.1, "motion": 0.3, "sleep": False},
        {"name": "Sleep", "hr": 55, "noise": 0.03, "motion": 0.0, "sleep": True},
        {"name": "Noisy", "hr": 75, "noise": 0.3, "motion": 0.1, "sleep": False},
    ]
    
    for i, scenario in enumerate(scenarios):
        print(f"\\nAnalyzing: {scenario['name']}")
        
        # Generate data
        data = generate_ppg_data(
            hr=scenario['hr'],
            noise=scenario['noise'], 
            motion=scenario['motion'],
            sleep=scenario['sleep'],
            duration=20
        )
        
        # Analyze
        stats, time, peaks = analyze_ppg_signal(data)
        
        # Create plot
        title = f"PPG Analysis: {scenario['name']} (HR: {scenario['hr']} bpm)"
        fig = plot_ppg_analysis(data, time, peaks, stats, title)
        
        # Save plot
        filename = f"ppg_analysis_{scenario['name'].lower().replace(' ', '_')}.png"
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"  Saved plot: {filename}")
        print(f"  Estimated HR: {stats['estimated_hr']:.1f} bpm (Target: {scenario['hr']} bpm)")
        
        plt.close(fig)
    
    print("\\n✅ Analysis complete! Check the generated PNG files.")

if __name__ == "__main__":
    main()
