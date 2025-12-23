import pandas as pd

# Load raw telemetry data
df = pd.read_csv("telemetry.csv")

# Convert timestamp
df["ts"] = pd.to_datetime(df["ts"])

# Sort by time
df = df.sort_values("ts")

# -------------------------------
# FEATURE ENGINEERING
# -------------------------------

# 1. Superheat (core refrigeration metric)
df["superheat"] = df["temperature"] - df["evap_temp"]

# 2. Temperature change rate
df["temp_delta"] = df["temperature"].diff()

# 3. Power per compressor RPM
df["power_per_rpm"] = df["power_watts"] / df["compressor_rpm"]

# 4. Fan efficiency
df["fan_efficiency"] = df["fan_rpm"] / df["power_watts"]

# 5. Vibration flag (basic anomaly indicator)
df["vibration_flag"] = df["vibration"].apply(lambda x: 1 if x > 0 else 0)

# -------------------------------
# CLEAN DATA
# -------------------------------

# Remove first row (diff creates NaN)
df = df.dropna()

# Replace infinities
df = df.replace([float("inf"), -float("inf")], 0)

# -------------------------------
# SAVE ML DATASET
# -------------------------------

df.to_csv("ml_dataset.csv", index=False)

print("✅ Phase 1 completed: ml_dataset.csv created")
