#!/usr/bin/env python3
from google import genai
import sys
import random

def openfile() -> str:
	with open("../../www/decouvrir.html", "r") as file:
		return file.read()

def openprompt() -> str:
	with open("prompt.txt", "r") as file:
		return file.read()

def cgi_error_response():
	sys.stdout.write(
		"Status: 500 Internal Server Error\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html><head><title>500 Internal Server Error</title></head>"
		"<body><h1>500 Internal Server Error</h1>"
		"<p>Une erreur est survenue lors de l'exécution du script CGI.</p>"
		"</body></html>"
	)

def main():
	lst_sign = ["Sagittaire", "Capricorne", "Verseau", "Poissons", "Bélier",
	"Taureau", "Gémeaux", "Cancer", "Lion", "Vierge", "Balance", "Scorpion"]
	signe = random.choice(lst_sign)
	file_content = openfile()
	client = genai.Client(api_key="AIzaSyAR47kNaGDqOc4Cd3XgPPXhXPJr7cULDDM")
	response = client.models.generate_content(
	model="gemini-2.5-flash",
	contents=openprompt().replace("{{signe}}", signe),
	)
	file_content = file_content.replace(
		"<!-- texte ici-->",
		"<p>" + response.text + "</p>"
	)
	sys.stdout.write("Content-Type: text/html\r\n\r\n")
	sys.stdout.write(file_content)

if __name__ == "__main__":
	try:
		main()

	except Exception as e:
		cgi_error_response()