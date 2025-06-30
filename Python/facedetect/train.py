import torch
import torch.nn as nn
import torch.nn.functional as F
import random
import os
import glob
import cv2
import torch.optim as optim
import numpy as np

class BinaryCNN(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 16, 3, padding=1)
        self.pool = nn.MaxPool2d(2)
        self.conv2 = nn.Conv2d(16, 32, 3, padding=1)
        self.fc1 = nn.Linear(32 * 16 * 16, 128)
        self.fc2 = nn.Linear(128, 1)

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))  # -> 32x32
        x = self.pool(F.relu(self.conv2(x)))  # -> 16x16
        x = x.view(x.size(0), -1)             # flatten
        x = F.relu(self.fc1(x))
        return self.fc2(x)  # raw logit
    
# Load model if exists

model=BinaryCNN()

optimizer=optim.Adam(model.parameters(), lr=1e-3)
if os.path.exists("model.pth"):
    model.load_state_dict(torch.load("model.pth"))
if os.path.exists("optimizer.pth"):
    optimizer.load_state_dict(torch.load("optimizer.pth"))

imgs=[os.path.basename(f) for f in glob.glob("face_img/faces_*.jpg")]
img_in=[torch.from_numpy((cv2.cvtColor(cv2.imread("face_img/"+f),cv2.COLOR_BGR2GRAY).astype(np.float32)/255-0.5)*2).unsqueeze(0) for f in imgs]
def read(fle):
    res=''
    with open(fle,'r') as file:
        res=file.read().strip(' \n')
    return res
img_out=[torch.tensor([float(read('labels/'+f+'.txt')=='True')]) for f in imgs]
ids=[i for i in range(len(imgs))]

criterion=nn.BCEWithLogitsLoss()
model.train()
while True:
    trainNm=int(input("How many times to train? "))
    bSz=int(input("Batch size? "))
    print("Number of times trained: 0",end="")
    ls=0
    for i in range(trainNm):
        optimizer.zero_grad()
        sample=random.sample(ids,bSz)
        sample_in=torch.stack([img_in[i] for i in sample])
        sample_out=torch.stack([img_out[i] for i in sample]).float()
        outputs=model(sample_in)
        loss=criterion(outputs,sample_out)
        ls=loss.item()
        loss.backward()
        optimizer.step()
        if i%100==0:
            print("\x1b["+str(len(str(i)))+"D"+str(i+1),end="")
    print("")
    print(f"Loss: {ls}")
    torch.save(model.state_dict(),"model.pth")
    torch.save(optimizer.state_dict(),"optimizer.pth")