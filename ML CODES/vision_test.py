import google.generativeai as genai
from PIL import Image

# CONFIGURE API KEY
genai.configure(api_key="AIzaSyDyR_8iwjpk_Uc4JQA-DoHREr3-2rmFm4w")

# LOAD MODEL
model = genai.GenerativeModel("gemini-1.5-flash")

# LOAD IMAGE
img = Image.open("icecream.jpg")

# ASK MODEL
response = model.generate_content(
    [
        "Describe what ice cream products or food items you see in this image.",
        img
    ]
)

print("MODEL OUTPUT:")
print(response.text)