import os
import requests

API_KEY = "<redacted>"
ROUTE_NAME = "myFiles"  # No need to create this beforehand

def upload_file(file_path):
    filename = os.path.basename(file_path)
    size = os.path.getsize(file_path)

    # Step 1: Request upload URL
    response = requests.post(
        "https://uploadthing.com/api/uploadFiles",
        headers={"x-uploadthing-api-key": API_KEY},
        json={
            "files": [{"name": filename, "size": size,"type":"application/octet-stream"}],
            "route": ROUTE_NAME,
        }
    )
    if response.status_code!=200:
        print(response.text)
    response.raise_for_status()
    data = response.json()["data"][0]
    upload_url = data["url"]
    file_key = data["key"]
    print(upload_url)
    print(data)

    # Step 2: Upload file
    with open(file_path, "rb") as f:
        files = {
            "file": (filename, f, "application/octet-stream")
        }
        upload = requests.post(upload_url, files=files,data=data["fields"])
        upload.raise_for_status()
        upload=requests.get(upload_url)
        print("Uploaded thing: "+upload.text)

    # Done!
    file_url = f"https://uploadthing.com/f/{file_key}"
    print("✅ Uploaded:", file_url)

upload_file("..\\yay.py")
def list_files():
    response = requests.post(
        "https://api.uploadthing.com/v6/listFiles",
        headers={"x-uploadthing-api-key": API_KEY,"Content-Type": "application/json"},
        json={}
    )
    print(response.text)
    response.raise_for_status()
    files = response.json()["files"]
    for f in files:
        print(f"📁 {f['name']} - size of {f['size']} (ID: {f['key']})")
list_files()