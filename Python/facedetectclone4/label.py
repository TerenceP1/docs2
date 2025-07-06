import glob
import os
import cv2
import numpy as np

folder = "face_img"
pattern = os.path.join(folder, "faces_*.jpg")  # matches faces_1.jpg, faces_abc.jpg, etc.

files_with_path = glob.glob(pattern)

queue=files_with_path
def pop():
    global queue
    res="dd"
    while os.path.exists("labels/"+res+".txt"):
        res=os.path.basename(queue.pop())
    return res
fle1=pop()
fle2=pop()
img1=cv2.imread("face_img/"+fle1)
img2=cv2.imread("face_img/"+fle2)
try:
    while len(queue)>0:
        print(len(queue))
        cv2.imshow("Label please (yn for left and 01 for right)",cv2.resize(np.concatenate((img1,img2), axis=1),(1024,512)))
        key=cv2.waitKey(0)
        if key in [48,49,110,121]:
            if key in [48,49]:
                with open(f"labels/{fle1}.txt",'w') as cfle:
                    cfle.write(str(key==49))
                fle1=pop()
                img1=cv2.imread("face_img/"+fle1)
            else:
                with open(f"labels/{fle2}.txt",'w') as cfle:
                    cfle.write(str(key==121))
                fle2=pop()
                img2=cv2.imread("face_img/"+fle2)
except Exception:
    cv2.destroyAllWindows()
    folder = "face_img"
    pattern = os.path.join(folder, "faces_*.jpg")  # matches faces_1.jpg, faces_abc.jpg, etc.

    files_with_path = glob.glob(pattern)
    for file in files_with_path:
        filenm=os.path.basename(file)
        if os.path.exists("labels/"+filenm+".txt"):
            continue
        img=cv2.imread(file)
        img=cv2.resize(img,(512,512))
        cv2.imshow(f"Is this the person (0 no and 1 for yes)? This is image {filenm}",img)
        key=0
        while key not in [48,49]:
            key=cv2.waitKey(0)
        with open("labels/"+filenm+".txt",'w') as fle:
            fle.write(str(key==49))
        cv2.destroyAllWindows()

cv2.destroyAllWindows()
folder = "face_img"
pattern = os.path.join(folder, "faces_*.jpg")  # matches faces_1.jpg, faces_abc.jpg, etc.

files_with_path = glob.glob(pattern)
for file in files_with_path:
    filenm=os.path.basename(file)
    if os.path.exists("labels/"+filenm+".txt"):
        continue
    img=cv2.imread(file)
    img=cv2.resize(img,(512,512))
    cv2.imshow(f"Is this the person (0 no and 1 for yes)? This is image {filenm}",img)
    key=0
    while key not in [48,49]:
        key=cv2.waitKey(0)
    with open("labels/"+filenm+".txt",'w') as fle:
        fle.write(str(key==49))
    cv2.destroyAllWindows()