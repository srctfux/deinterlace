
This is a sample app that plays movie files and renders them thru the deinterlace dmo filter

Thers not much error checking in the code, so it might crash.

How it works:
select File - Open file in the menu and select a movie file supported by direct show
if everything works, the program will build a direct show graph.

at this stage if you want to you can start graphedit from directx sdk or platform sdk
and view/examine the graph.

to get the movie playing select Controls and then play, now the movie shod start playing

the program uses overlay to play the file, so your grafic card must support overlay for the program to work.
the program shoud be able to survive a screen resolution change and/or color depth change.
