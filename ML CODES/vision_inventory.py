import time
import google.generativeai as genai
from PIL import Image
import os

# ============================
# CONFIG
# ============================
API_KEY = "AIzaSyDyR_8iwjpk_Uc4JQA-DoHREr3-2rmFm4w"
IMAGE_FILE = "icecream.jpg"

# ============================
# INIT
# ============================
genai.configure(api_key=API_KEY)

model = genai.GenerativeModel(
    "models/gemini-2.0-flash-exp-image-generation"
)

# ============================
# CHECK IMAGE
# ============================
if not os.path.exists(IMAGE_FILE):
    print("❌ Image not found:", IMAGE_FILE)
    exit()

img = Image.open(IMAGE_FILE)

# ============================
# PROMPT
# ============================
prompt = """
You are an inventory detection system for a freezer.

Look at the image and count ice cream packets by flavor.

Return ONLY in this exact format:
Vanilla: <number>
Chocolate: <number>
Strawberry: <number>
Other: <number>

Do not explain anything.
"""

# ============================
# AI CALL
# ============================
print("⏳ Processing image...")
time.sleep(5)  # avoid quota issues

response = model.generate_content(
    [prompt, img],
    generation_config={
        "temperature": 0.2,
        "max_output_tokens": 200
    }
)

# ============================
# OUTPUT
# ============================
print("\n✅ INVENTORY RESULT")
print("-------------------")
print(response.text)