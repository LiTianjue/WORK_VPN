@echo off
if x%1 == x goto end
if NOT %1 == Select_Cert goto end

rem ����ʵ������޸����¸��е�·��
if NOT EXIST "%~sdp0..\bin\Select_Cert.exe" goto end
 "%~sdp0..\bin\Select_Cert.exe"
if NOT ERRORLEVEL 1 goto end
 echo  --pkcs12 "%~sdp0client.pfx"

:end
puause