#!/bin/sh 

#DScaler Site Update Script
#Dan Schmelzer
#dschmelzer@gmail.com
#2/18/2008

#Instructions for running

#copy this script onto your personal shell space on Sourceforge
#then type
#sh helpup.sh <Enter>

#Then check out htdocs directory from SVN.  Only needs to be run once, when
#htdocs directory is blank.

#cd /home/groups/d/de/deinterlace/
#svn co https://deinterlace.svn.sourceforge.net/svnroot/deinterlace/trunk/htdocs

#Note re Sourceforge permissions...
#All of the files in htdocs will be assigned to you as the user.  If somebody
#in the group has already populated the htdocs directory from SVN, but you
#want to update the web directory yourself, then you will have to ask them
#to delete all of the files in the htdocs directory assigned to them.
#Likewise, if you have populated the htdocs directory from SVN, others will
#not be able to update the web directory.  This is a long-standing permissions
#issue with Sourceforge.

#After the first run, merely have SVN update the already checked out directory

cd /home/groups/d/de/deinterlace/htdocs
svn update
