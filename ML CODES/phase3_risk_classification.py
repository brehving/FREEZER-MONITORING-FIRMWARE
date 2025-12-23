import pandas as pd

# ---------------------------------------
# LOAD DATA (FROM STEP 1)
# ---------------------------------------
df = pd.read_csv("ml_dataset_with_health_score.csv")

# ---------------------------------------
# RISK CLASSIFICATION FUNCTION
# ---------------------------------------
def classify_risk(score):
    if score >= 80:
        return "NORMAL"
    elif score >= 50:
        return "WARNING"
    else:
        return "CRITICAL"

def action_for_risk(risk):
    if risk == "NORMAL":
        return "No action needed"
    elif risk == "WARNING":
        return "Monitor system & schedule inspection"
    else:
        return "Immediate maintenance required"

# ---------------------------------------
# APPLY CLASSIFICATION
# ---------------------------------------
df["risk_level"] = df["health_score"].apply(classify_risk)
df["recommended_action"] = df["risk_level"].apply(action_for_risk)

# ---------------------------------------
# SAVE OUTPUT
# ---------------------------------------
df.to_csv("ml_dataset_with_risk_levels.csv", index=False)

print("✅ Phase 3 – Step 2 completed")
print("📁 Output file: ml_dataset_with_risk_levels.csv")

# Quick summary
print("\nRisk distribution:")
print(df["risk_level"].value_counts())
