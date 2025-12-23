import google.generativeai as genai

genai.configure(api_key="AIzaSyDyR_8iwjpk_Uc4JQA-DoHREr3-2rmFm4w")

for model in genai.list_models():
    print(model.name)