
DeinterlaceDMOps.dll: dlldata.obj DeinterlaceDMO_p.obj DeinterlaceDMO_i.obj
	link /dll /out:DeinterlaceDMOps.dll /def:DeinterlaceDMOps.def /entry:DllMain dlldata.obj DeinterlaceDMO_p.obj DeinterlaceDMO_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del DeinterlaceDMOps.dll
	@del DeinterlaceDMOps.lib
	@del DeinterlaceDMOps.exp
	@del dlldata.obj
	@del DeinterlaceDMO_p.obj
	@del DeinterlaceDMO_i.obj
