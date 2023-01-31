@echo off
set imageFolder="%~1"
cd ..
set exeFolder=%cd%\exe\wsi-anon.exe
forfiles /P "%imageFolder%" /c "cmd /c echo F|%exeFolder% @FILE -n anonymized_@FNAME"