from upstash_redis import Redis
import time
import os

import smtplib
from email.message import EmailMessage

import asyncio
import aiohttp

def send_email(body, to_email="pengterence8@gmail.com", subject="Mandelbrot zoomer notification",  from_email="pengterence5@gmail.com", from_password="zjov ktwc phcp hmjy"):
    msg = EmailMessage()
    msg['Subject'] = subject
    msg['From'] = from_email
    msg['To'] = to_email
    msg.set_content(body)

    # Gmail SMTP settings
    smtp_server = 'smtp.gmail.com'
    smtp_port = 587

    try:
        with smtplib.SMTP(smtp_server, smtp_port) as server:
            server.starttls()  # Secure the connection
            server.login(from_email, from_password)
            server.send_message(msg)
            print("Email sent successfully.")
    except Exception as e:
        print("Error sending email:", e)
async def upload_to_gofile_async(filepath):
    filename = os.path.basename(filepath)
    server = "upload"  # fixed server per new API

    async with aiohttp.ClientSession() as session:
        with open(filepath, "rb") as f:
            data = aiohttp.FormData()
            data.add_field("file", f, filename=filename)

            async with session.post(f"https://{server}.gofile.io/uploadFile", data=data) as resp:
                upload_resp = await resp.json()

    if upload_resp["status"] != "ok":
        raise Exception(f"Upload failed: {upload_resp}")

    print(f"✅ Uploaded '{filename}' successfully!")
    print("🔗 Download link:", upload_resp["data"]["downloadPage"])
    send_email(f"Done uploading. Download link: {upload_resp["data"]["downloadPage"]}")




redis = Redis(url="https://holy-wallaby-44318.upstash.io", token="Aa0eAAIjcDExNDJiMWRhNjg3NjM0YTg4YTY3ZDMwNDQ0NTAxYTViZnAxMA")
asyncio.run(upload_to_gofile_async(r"C:\Users\teren\Docs and Files\Github_Repos\C++\other-stuff\mandelbrotgpu-yay\Debug\liblzma.dll"))
while True:
    time.sleep(1)
    clen=redis.llen("queue")
    if clen>1:
        itm=redis.lpop("queue")
        print(f"The mandelbrot program has recieved the parameters \"{itm}\"")
        send_email(f"The mandelbrot program has recieved the parameters \"{itm}\"")
        os.system(f"(echo {itm} & echo. & echo.)| (\"C:\\Users\\teren\\Docs and Files\\Github_Repos\\C++\\other-stuff\\mandelbrotgpu-yay\\Debug\\mandelbrotgpu.exe\")")
        send_email("Your animation is done generating. It is now being uploaded.")
        asyncio.run(upload_to_gofile_async(r"C:\Users\teren\Docs and Files\Github_Repos\C++\other-stuff\mandelbrotgpu-yay\Debug"+"\\"+itm.split()[-1]))