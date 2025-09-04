/*
 * PPG Signal Simulator - Host Version
 * 
 * Host-compatible version without Zephyr dependencies
 */

#ifndef PPG_SIMULATOR_HOST_H
#define PPG_SIMULATOR_HOST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Configuration limits */
#define PPG_SIM_MIN_HR          40      /* Minimum heart rate (bpm) */
#define PPG_SIM_MAX_HR          200     /* Maximum heart rate (bpm) */
#define PPG_SIM_DEFAULT_HR      70      /* Default heart rate (bpm) */

/* Utility macros */
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Simulator configuration structure */
struct ppg_sim_config {
    float heart_rate_bpm;       /* Heart rate in beats per minute */
    float noise_level;          /* Noise level (0.0 - 1.0) */
    float motion_artifacts;     /* Motion artifact level (0.0 - 1.0) */
    int sleep_mode;             /* Sleep mode flag */
    float breathing_rate_bpm;   /* Breathing rate in breaths per minute */
    uint8_t signal_quality;     /* Signal quality indicator (0-100) */
};

/* Simulator internal state */
static struct ppg_sim_state {
    uint32_t sample_count;
    float last_timestamp_s;
    float phase_offset;
    int initialized;
    struct ppg_sim_config config;
} sim_state = {0};

/* Function prototypes */
int ppg_sim_init(const struct ppg_sim_config *config);
float ppg_sim_generate_sample(uint32_t timestamp_ms);
float ppg_sim_generate_heartbeat(float t, float hr_hz);
float ppg_sim_add_breathing_modulation(float t);
float ppg_sim_add_noise(void);
float ppg_sim_add_motion_artifacts(float t);

/* Implementation */
int ppg_sim_init(const struct ppg_sim_config *config)
{
    if (config) {
        sim_state.config = *config;
    } else {
        // Default configuration
        sim_state.config.heart_rate_bpm = PPG_SIM_DEFAULT_HR;
        sim_state.config.noise_level = 0.1f;
        sim_state.config.motion_artifacts = 0.0f;
        sim_state.config.sleep_mode = 0;
        sim_state.config.breathing_rate_bpm = 16.0f;
        sim_state.config.signal_quality = 95;
    }
    
    sim_state.sample_count = 0;
    sim_state.last_timestamp_s = 0.0f;
    sim_state.phase_offset = 0.0f;
    sim_state.initialized = 1;
    
    printf("PPG Simulator initialized - HR: %.1f bpm, Noise: %.2f\n", 
           sim_state.config.heart_rate_bpm, sim_state.config.noise_level);
    
    return 0;
}

float ppg_sim_generate_sample(uint32_t timestamp_ms)
{
    if (!sim_state.initialized) {
        ppg_sim_init(NULL);
    }
    
    float t = timestamp_ms / 1000.0f;
    float dt = t - sim_state.last_timestamp_s;
    sim_state.last_timestamp_s = t;
    
    /* Heart rate frequency */
    float hr_hz = sim_state.config.heart_rate_bpm / 60.0f;
    
    /* Generate realistic PPG waveform */
    float ppg_base = ppg_sim_generate_heartbeat(t, hr_hz);
    
    /* Add breathing modulation */
    float breathing_mod = ppg_sim_add_breathing_modulation(t);
    
    /* Add noise */
    float noise = ppg_sim_add_noise();
    
    /* Add motion artifacts if enabled */
    float motion = ppg_sim_add_motion_artifacts(t);
    
    /* Combine all components */
    float final_signal = ppg_base + breathing_mod + noise + motion;
    
    /* Ensure signal stays in valid range [0.0, 1.0] */
    final_signal = CLAMP(final_signal, 0.0f, 1.0f);
    
    sim_state.sample_count++;
    
    return final_signal;
}

float ppg_sim_generate_heartbeat(float t, float hr_hz)
{
    /* Generate realistic PPG heartbeat waveform */
    float phase = 2.0f * M_PI * hr_hz * t + sim_state.phase_offset;
    
    /* Primary heartbeat component (systolic peak) */
    float heartbeat = 0.7f + 0.2f * sin(phase);
    
    /* Add dicrotic notch (secondary peak) */
    float dicrotic_phase = phase + M_PI * 0.3f;
    heartbeat += 0.05f * sin(dicrotic_phase);
    
    /* Add some harmonic content for realism */
    heartbeat += 0.02f * sin(2.0f * phase);
    heartbeat += 0.01f * sin(3.0f * phase);
    
    /* Sleep mode adjustment */
    if (sim_state.config.sleep_mode) {
        heartbeat *= 0.85f; // Reduced amplitude during sleep
    }
    
    return heartbeat;
}

float ppg_sim_add_breathing_modulation(float t)
{
    /* Breathing rate in Hz */
    float breathing_hz = sim_state.config.breathing_rate_bpm / 60.0f;
    
    /* Breathing modulation affects baseline */
    float breathing_phase = 2.0f * M_PI * breathing_hz * t;
    float modulation = 0.03f * sin(breathing_phase);
    
    /* Stronger modulation during sleep */
    if (sim_state.config.sleep_mode) {
        modulation *= 1.5f;
    }
    
    return modulation;
}

float ppg_sim_add_noise(void)
{
    if (sim_state.config.noise_level <= 0.0f) {
        return 0.0f;
    }
    
    /* Generate white noise */
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    noise *= sim_state.config.noise_level * 0.1f;
    
    return noise;
}

float ppg_sim_add_motion_artifacts(float t)
{
    if (sim_state.config.motion_artifacts <= 0.0f) {
        return 0.0f;
    }
    
    /* Generate motion artifacts as low-frequency interference */
    float motion_freq = 0.5f + ((float)rand() / RAND_MAX) * 2.0f; // 0.5-2.5 Hz
    float motion_phase = 2.0f * M_PI * motion_freq * t;
    
    float artifact = sim_state.config.motion_artifacts * 0.2f * sin(motion_phase);
    
    /* Add some random spikes */
    if ((rand() % 1000) < 5) { // 0.5% chance
        artifact += sim_state.config.motion_artifacts * 0.3f * ((float)rand() / RAND_MAX - 0.5f);
    }
    
    return artifact;
}

#endif /* PPG_SIMULATOR_HOST_H */
