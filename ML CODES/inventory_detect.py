import google.generativeai as genai
from PIL import Image

# CONFIGURE API
genai.configure(api_key="AIzaSyDyR_8iwjpk_Uc4JQA-DoHREr3-2rmFm4w")

# LOAD IMAGE
img = Image.open("icecream.jpg")

# MODEL
model = genai.GenerativeModel("gemini-2.0-flash")

# PROMPT (STRICT – NO GUESSING)
prompt = """
You are an inventory detection system.

ONLY detect flavours that are CLEARLY written on the ice cream package.
DO NOT guess flavours.
DO NOT assume vanilla or chocolate unless visible.

Return output in this exact format:

Flavours:
- flavour_name

Count each flavour assuming one pack per visible item.
"""

# GENERATE
response = model.generate_content([prompt, img])

# OUTPUT
print("\n===== INVENTORY ANALYSIS =====")
print(response.text)
print("================================")