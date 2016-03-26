@echo off
start .\Debug\server.exe
echo Server started
for /L %%n in (1,1,10) do (
	start /MIN .\Debug\client.exe --log_file=client_%%n.log
	echo Started client #%%n
)
echo Ctr+C in 'server.exe' window" to gracefully stop everything
pause

