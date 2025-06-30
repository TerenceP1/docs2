import cv2
import numpy as np
import os

# Huge thanks to ChatGPT for speeding it up

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

    # Normalize pixel values to [0,1] (NO)
    #normalized_img = resized_img.astype(np.float32) / 255.0

    return resized_img

# Begin collecting faces

face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)

def get_next_available_filename(folder=".", prefix="faces_", ext=".jpg"):
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
        fle=get_next_available_filename(folder="face_img")
        cv2.imwrite(fle,norm)
        cv2.imshow("Face",cv2.resize(norm,(512,512)))
        cv2.waitKey(1)