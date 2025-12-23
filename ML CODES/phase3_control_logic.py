import pandas as pd

# =========================
# FILE PATHS
# =========================
INPUT_FILE  = "ml_dataset_with_anomalies.csv"
OUTPUT_FILE = "phase3_control_output.csv"

# =========================
# LOAD DATA
# =========================
df = pd.read_csv(INPUT_FILE)

# =========================
# DEFAULT SAFE VALUES
# =========================
DEFAULT_EEV_STEP = 50
DEFAULT_FAN_RPM = 1200
DEFAULT_COMP_RPM = 1800

# =========================
# ACTION LOGIC FUNCTION
# =========================
def decide_control(row):
    eev = DEFAULT_EEV_STEP
    fan = DEFAULT_FAN_RPM
    comp = DEFAULT_COMP_RPM
    reason = "Normal operation"

    # POWER anomaly → reduce load
    if row.get("power_watts", 0) > 900:
        comp -= 300
        fan += 200
        reason = "High power detected → reducing compressor load"

    # SUPERHEAT high → open EEV
    if row.get("superheat", 0) > 12:
        eev += 10
        fan += 100
        reason = "High superheat → opening EEV"

    # TEMPERATURE not dropping → boost cooling
    if row.get("temp_delta", 0) > 3:
        comp += 200
        fan += 200
        reason = "Poor cooling → increasing RPM"

    # VIBRATION anomaly → safety reduction
    if row.get("vibration", 0) > 1.5:
        comp -= 500
        fan -= 200
        reason = "Vibration anomaly → safety throttle"

    return pd.Series([
        max(20, min(100, eev)),
        max(800, min(2000, fan)),
        max(1000, min(2500, comp)),
        "OPEN" if eev > DEFAULT_EEV_STEP else "HOLD",
        "HIGH" if fan > DEFAULT_FAN_RPM else "NORMAL",
        "REDUCED" if comp < DEFAULT_COMP_RPM else "NORMAL",
        reason
    ])

# =========================
# APPLY CONTROL LOGIC
# =========================
df[[
    "eev_step_cmd",
    "fan_rpm_cmd",
    "compressor_rpm_cmd",
    "eev_state",
    "fan_state",
    "compressor_state",
    "control_reason"
]] = df.apply(decide_control, axis=1)

# =========================
# SAVE OUTPUT
# =========================
df.to_csv(OUTPUT_FILE, index=False)

print("✅ Phase-3 Control Logic Completed")
print(f"📁 Output saved as: {OUTPUT_FILE}")
