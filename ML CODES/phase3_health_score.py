import pandas as pd

# ---------------------------------------
# LOAD DATA
# ---------------------------------------
df = pd.read_csv("ml_dataset_with_anomalies.csv")

# ---------------------------------------
# AUTO-DETECT ANOMALY COLUMNS
# (anything containing 'anomaly')
# ---------------------------------------
anomaly_cols = [c for c in df.columns if "anomaly" in c.lower()]

print("Detected anomaly columns:")
print(anomaly_cols)

# ---------------------------------------
# NORMALIZE VALUES
# IsolationForest: -1 = anomaly, 1 = normal
# Convert to 1 = anomaly, 0 = normal
# ---------------------------------------
for col in anomaly_cols:
    if df[col].dtype != object:
        df[col] = df[col].apply(lambda x: 1 if x == -1 else 0)
    else:
        df[col] = df[col].apply(lambda x: 1 if str(x).lower() == "anomaly" else 0)

# ---------------------------------------
# ANOMALY COUNT
# ---------------------------------------
df["anomaly_count"] = df[anomaly_cols].sum(axis=1)

# ---------------------------------------
# HEALTH SCORE (0–100)
# ---------------------------------------
MAX_ANOMALIES = len(anomaly_cols)

df["health_score"] = 100 - (df["anomaly_count"] / MAX_ANOMALIES) * 100
df["health_score"] = df["health_score"].round(2)

# ---------------------------------------
# SAVE OUTPUT
# ---------------------------------------
df.to_csv("ml_dataset_with_health_score.csv", index=False)

print("\n✅ Phase 3 Step 1 completed")
print("📁 Output file: ml_dataset_with_health_score.csv")
