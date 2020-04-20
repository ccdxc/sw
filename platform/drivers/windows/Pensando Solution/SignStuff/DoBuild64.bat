call sign "Pensando Solution\x64\release\ionic64.sys
rmdir /s/q cat64
mkdir cat64
copy "Pensando Solution\x64\release\ionic64.sys" cat64\.
copy "Pensando Solution\x64\release\ionic64.inf" cat64\.
"C:\Program Files (x86)\Windows Kits\10\bin\x86\inf2cat.exe" /driver:cat64 /os:8_X64,Server6_3_X64,Server8_X64,10_X64,10_RS2_X64,10_RS3_X64,10_RS4_X64,Server10_X64,Server2016_X64,ServerRS2_X64,ServerRS3_X64,ServerRS4_X64
.\sign.bat cat64\ionic64.cat
:end
