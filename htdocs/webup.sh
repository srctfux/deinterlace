#!/bin/sh 

cd /home/groups/deinterlace/ 
rm -R htdocs/*
cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace htdocs >/dev/null
cd /home/groups/deinterlace/htdocs/distributions
wget http://www.schmelzer.org/dtvupload/dTV18exe.zip
http://www.schmelzer.org/dtvupload/dTV18src.zip
