cd %~dp0

rmdir /s/q cab64
mkdir cab64
cd cab64
makecab /f ..\CabDesc64.ddf
cd ..
.\sign.bat cab64\disk1\Pensando_Driver64.cab