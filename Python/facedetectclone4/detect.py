import cv2
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
import os
import pygame
pygame.mixer.init()
pygame.mixer.music.load(r"..\louder.mp3")

class BinaryCNN(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 16, 3, padding=1)
        self.pool = nn.MaxPool2d(2)
        self.conv2 = nn.Conv2d(16, 32, 3, padding=1)
        self.fc1 = nn.Linear(32 * 16 * 16, 128)
        self.fc2 = nn.Linear(128, 1)
        self.dropout = nn.Dropout(p=0.5)  # 50% dropout rate

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))  # -> 32x32
        x = self.pool(F.relu(self.conv2(x)))  # -> 16x16
        x = x.view(x.size(0), -1)             # flatten
        x = F.relu(self.fc1(x))
        x = self.dropout(x) 
        return self.fc2(x)  # raw logit
    
model=BinaryCNN()
model.load_state_dict(torch.load("model.pth"))
model.eval()
cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
def square_and_preprocess_face(face_img, target_size=64):
    """
    Takes a cropped face image, pads it with zeros to make it square,
    normalizes pixel values to 0-1, and resizes to target_size x target_size.

    Args:
        face_img (np.ndarray): Cropped face image, shape (H, W) or (H, W, C).
        target_size (int): Size to resize the squared image to (default 64).

    Returns:
        np.ndarray: Preprocessed image of shape (target_size, target_size, C) with float32 values 0-1.
    """
    h, w = face_img.shape[:2]

    # Determine padding amounts
    if h == w:
        squared_img = face_img
    else:
        # Calculate padding to add on each side
        size = max(h, w)
        top = bottom = left = right = 0

        if h < size:
            diff = size - h
            top = diff // 2
            bottom = diff - top
        if w < size:
            diff = size - w
            left = diff // 2
            right = diff - left

        # Pad with zeros (black)
        squared_img = cv2.copyMakeBorder(face_img, top, bottom, left, right,
                                         borderType=cv2.BORDER_CONSTANT, value=0)

    # Resize to target size (64x64)
    resized_img = cv2.resize(squared_img, (target_size, target_size), interpolation=cv2.INTER_AREA)

    # Normalize pixel values to [0,1]
    normalized_img = (resized_img.astype(np.float32) / 255.0 -0.5)*2.0

    return normalized_img
def get_next_available_filename(folder=".", prefix="hehe_", ext=".jpg"):
    i = 1
    while True:
        filename = f"{prefix}{i}{ext}"
        filepath = os.path.join(folder, filename)
        if not os.path.exists(filepath):
            return filepath
        i += 1
while True:
    print("Detecting")
    _,img=cap.read()
    img=cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(img, scaleFactor=1.1, minNeighbors=5)
    for x,y,w,h in faces:
        print("face")
        face_crop = img[y:y+h, x:x+w]
        norm=square_and_preprocess_face(face_crop)
        inp=torch.stack([torch.from_numpy((norm)).float().unsqueeze(0)])
        outputs=model(inp)
        print(norm.shape, norm.min(), norm.max())
        if torch.sigmoid(outputs).item()>0.7:
        #if True:
            #os.system("powershell -c (New-Object Media.SoundPlayer '..\louder.wav').PlaySync();")
            if not pygame.mixer.music.get_busy():
                pygame.mixer.music.play()
            cv2.imshow("Face",cv2.resize(np.clip(((norm+1)/2*255),0,255).astype(np.uint8),(512,512)))
        cv2.waitKey(1)
        cv2.imwrite(get_next_available_filename(folder='detect'),np.clip(((norm+1)/2*255),0,255).astype(np.uint8))
    cv2.waitKey(1)