#!/usr/bin/python

import os,struct,sys,codecs,time
from stat import *
try:
    import psyco
    psyco.full()
except: pass

# Settings here
processmaplist=[0,1,30,34,36,43,47,48,70,90,109,129,189,209,229,230,249,269,289,309,
    329,369,389,409,429,449,509,530,531,532]
rootpath="./../../"
StrtCoords="StartCoords"
vmaps="demodata/vmaps"
mmaps="mmaps"
waittime=0.8

# Initialization of variables used
gridlist=[]
dones=[]
startCoords=[]
failed=[]
exportmap=0

# Functions
def GetSplit(part1,reverse):
    coords=part1.split('_')
    imap=int(coords[0])
    try:
        x=int(coords[1])
        y=int(coords[2])
    except:
        x=0
        y=0
    if imap==exportmap:
        if reverse==1:
            grid=(y,x)
        else:
            grid=(x,y)
        return grid

def sortbysize(filelist):
    tmplist=[]
    for one in filelist:
        tmplist.append((os.stat(rootpath+StrtCoords+"/"+one)[ST_SIZE],one))
    tmplist.sort(key=lambda x:x[0],reverse=True)
    flist=[]
    for one in tmplist:
        flist.append(one[1])
    return flist
    
def updateList(path,sorting):
    ilist=[]
    osdones=os.listdir(path)
    if sorting:
        osdones=sortbysize(osdones)
    for one in osdones:
        try:
            name=one.split(".")[0]
            imap=int(name.split("_")[0])
            if imap==exportmap:
                ilist.append(GetSplit(name,0))
        except:
            continue
    return ilist

def oneleft():
    for one in startCoords:
        if dones.count(one)==0 and failed.count(one)==0 and gridlist.count(one)>0:
            return one
    return 0

# Main code
for exportmap in processmaplist:
    fulllist = os.listdir(rootpath+vmaps)
    for one in fulllist:
        f=one.split(".")
        ext=f[1]
        imap=int(f[0].split("_")[0])
        if ext=="vmdir" and imap==exportmap:
            gridlist.append(GetSplit(f[0],0))
    fulllist = os.listdir(rootpath+StrtCoords)
    for one in fulllist:
        try:
            f=one.split(".")
            ext=f[1]
            imap=int(f[0].split("_")[0])
            if ext=="txt" and imap==exportmap:
                gridlist.append(GetSplit(f[0],0))
        except:
            continue

    goon=1
    past=0
    dones=updateList(rootpath+mmaps,0)
    startCoords=updateList(rootpath+StrtCoords,1)
    while goon:
        gridleft=oneleft()
        if gridleft:
            if gridleft==past:
                failed.append(gridleft)
            else:
                past=gridleft
                print "\nGenerating "+str(exportmap)+" "+str(gridleft[0])+" "+str(gridleft[1])
                os.system("Generator "+str(exportmap)+" "+str(gridleft[0])+" "+str(gridleft[1])+" 1")
                time.sleep(waittime)
        else:
            goon=0
        dones=updateList(rootpath+mmaps,0)
        startCoords=updateList(rootpath+StrtCoords,1)
    
    file=open('failed'+str(exportmap)+'.txt','w')
    for one in failed:
        if one[0]!=0 and one[1]!=0:
            file.write(str(one[0])+","+str(one[1])+'\n')
    for one in gridlist:
        if dones.count(one)==0:
            if one[0]!=0 and one[1]!=0:
                file.write(str(one[0])+","+str(one[1])+'\n')
    file.close()
