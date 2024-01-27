@echo off

echo This will try to compile an optimize LibSass version
echo You must pass exactly three arguments or funky stuff will happen
echo Usage: train [x86/x64] [Shared/Static] [Release/Debug]

git clone "https://github.com/mgreter/libsass-specs" --branch bolt-bench build/libsass-spec

echo Going to build libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /t:Clean;Build /p:PGO="None"
MSBuild libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /t:Clean;Build /p:PGO="None"

echo ######################################################
echo Get base time for untrained "%1\%3\%2\trainer.exe"
cd build
echo ######################################################
FOR /L %%A IN (1,1,10) DO (
  %1\%3\%2\trainer.exe libsass-spec\suites\bolt-bench\input.scss 1>nul
)
echo ######################################################
cd ..

echo Going to build libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /t:Clean;Build /p:PGO="Instrument"
MSBuild libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /t:Clean;Build /p:PGO="Instrument"

echo ######################################################
echo Collecting data for "%1\%3\%2\trainer.exe"
cd build
echo ######################################################
FOR /L %%A IN (1,1,3) DO (
  %1\%3\%2\trainer.exe libsass-spec\suites\bolt-bench\input.scss 1>nul
)
echo ######################################################
cd ..

echo Going to build libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /p:PGO="Optimize"
MSBuild libsass.sln /p:Platform="%1" /p:Configuration="%3 %2" /p:PGO="Optimize"

echo ######################################################
echo Get timing after optimizations "%1\%3\%2\trainer.exe"
cd build
echo ######################################################
FOR /L %%A IN (1,1,10) DO (
  %1\%3\%2\trainer.exe libsass-spec\suites\bolt-bench\input.scss 1>nul
)
echo ######################################################
cd ..
