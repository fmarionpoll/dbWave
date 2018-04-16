cd /d "C:\Users\fred\source\repos\dbWave2\dbWave2" &msbuild "dbWave2.vcxproj" /t:sdvViewer /p:configuration="Debug" /p:platform=Win32
exit %errorlevel% 