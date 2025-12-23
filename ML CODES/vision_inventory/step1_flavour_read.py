import google.generativeai as genai
from PIL import Image

# ---- API KEY ----
genai.configure(api_key="AIzaSyDyR_8iwjpk_Uc4JQA-DoHREr3-2rmFm4w")

# ---- MODEL (FIXED) ----
model = genai.GenerativeModel("models/gemini-pro-vision")

# ---- LOAD IMAGE ----
img = Image.open("test.jpg")

# ---- PROMPT ----
prompt = """
Look at this image.
Tell me what ice cream flavours you see.
Just explain in words.
"""

# ---- RUN ----
response = model.generate_content([prompt, img])

print("AI OUTPUT:")
print(response.text)