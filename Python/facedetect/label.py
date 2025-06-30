import glob
import os
import cv2

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