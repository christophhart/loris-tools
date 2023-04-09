echo loris_library Build Script
echo ==============================================================
echo Cleanup...

rm -rf loris_library/Builds
rm loris_library_macOS.zip

export loris_version=$(git describe --tags --abbrev=0)
echo Library version is $loris_version

chmod +x "/Applications/Projucer.app/Contents/MacOS/Projucer"

"/Applications/Projucer.app/Contents/MacOS/Projucer" --resave "loris_library/loris_library.jucer"

set -o pipefail
echo Compiling Loris Dynamic Library...

echo Compile Debug Configuration
xcodebuild -project ./loris_library/Builds/MacOSX/loris_library.xcodeproj -configuration "Debug" -jobs "6" | xcpretty

echo Compile Release Configuration...

xcodebuild -project ./loris_library/Builds/MacOSX/loris_library.xcodeproj -configuration "Release" -jobs "6" | xcpretty

echo Creating Archive...
zip -X -j loris_library_macOS.zip ./loris_library/Builds/MacOSX/build/Debug/loris_library_debug.dylib ./loris_library/Builds/MacOSX/build/Release/loris_library_release.dylib