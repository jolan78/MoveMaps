#!/usr/bin/python

import os,struct,sys,codecs
try:
    import psyco
    psyco.full()
except: pass
try:
    import MySQLdb
except Exception,detail:
    print detail
    raise SystemExit
from math import sqrt

# Configuration
#                   IP     port user   password  db
MangosDatabaseInfo="dbserv;3306;mangos;mypass;mangos"
MangosDbInfo=MangosDatabaseInfo.split(";")

try:
    MangosDb = MySQLdb.connect(host=MangosDbInfo[0], port=int(MangosDbInfo[1]), user=MangosDbInfo[2], passwd=MangosDbInfo[3],db=MangosDbInfo[4], init_command='SET NAMES utf8')
except:
    print "Enable to connect to database on Host "+'"'+MangosDbInfo[0]+'" with:'
    print ' Port: '+MangosDbInfo[1]
    print ' Username: '+MangosDbInfo[2]
    print ' Password: '+MangosDbInfo[3]
    print ' Database: '+MangosDbInfo[4]
    raise SystemExit
MangosDbCursor = MangosDb.cursor()

def computevmapgridpair(x,y):
    SIZE_OF_GRIDS=533.33333
    CENTER_GRID_OFFSET=SIZE_OF_GRIDS/2
    CENTER_GRID_ID=64/2
    x_offset = (x - CENTER_GRID_OFFSET)/SIZE_OF_GRIDS
    y_offset = (y - CENTER_GRID_OFFSET)/SIZE_OF_GRIDS
    x_val = int(x_offset+CENTER_GRID_ID + 0.5);
    y_val = int(y_offset+CENTER_GRID_ID + 0.5);
    return (63 - x_val,63 - y_val)
    
print "Working..."
query="select map,position_x,position_y,position_z from creature where id not in (select entry from creature_template where InhabitType>3)"
MangosDbCursor.execute(query)
if int(MangosDbCursor.rowcount)>0:
    result=MangosDbCursor.fetchall()
    for record in result:
        grid=computevmapgridpair(record[1],record[2])
        file=open('StartCoords/%03d_%d_%d.txt' % (record[0],grid[0],grid[1]),'ab')
        file.write("%f,%f,%f\n" % (record[1],record[2],record[3]))
        file.close()
print "Done..."
