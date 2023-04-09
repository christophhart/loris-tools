@echo off
echo Build Loris Library Windows
git describe --abbrev=0 > tmpFile

SET /p versionPoint= < tmpFile

echo Version %versionPoint%


set projucer="D:\Development\HISE modules\tools\projucer\Projucer.exe"
set zip_app="C:\Program Files\7-Zip\7z.exe"
set compiler="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MsBuild.exe"

%projucer% --resave "loris_library/loris_library.jucer"

set dll_project=loris_library/Builds/VisualStudio2017/loris_library.sln

echo Build Debug Configuration...

%compiler%  %dll_project% /t:Build /p:Configuration="Debug";Platform=x64 /v:m

echo Build Release Configuration...

%compiler% %dll_project% /t:Build /p:Configuration="Release";Platform=x64 /v:m

echo Zipping Archive

%zip_app% a "loris_library_win64.zip" ".\loris_library\Builds\VisualStudio2017\x64\Debug\Dynamic Library\loris_library_debug.dll"
%zip_app% a "loris_library_win64.zip" ".\loris_library\Builds\VisualStudio2017\x64\Release\Dynamic Library\loris_library_release.dll"

pause