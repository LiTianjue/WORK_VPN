@echo off
cls

ver | find "4.0." > NUL &&  goto win95  
ver | find "4.10." > NUL &&  goto win98 
ver | find "4.90." > NUL &&  goto win_me 
ver | find "3.51." > NUL &&  goto win_Nt_3_5 
ver | find "5.0." > NUL &&  goto win2000  
ver | find "5.1." > NUL &&  goto win_xp  
ver | find "5.2." > NUL &&  goto win2003  
ver | find "6.0." > NUL &&  goto vista 
ver | find "6.1." > NUL &&  goto win7  
ver | find "6.2." > NUL &&  goto win8  

:win95
@echo           当前系统是：win95
@echo ----------------------------------------
goto end

:win98
@echo           当前系统是：win98
@echo ----------------------------------------
goto end

:win_me
@echo           当前系统是：win_me
@echo ----------------------------------------
goto end

:win_Nt_3_5
@echo           当前系统是：win NT 3.51
@echo ----------------------------------------
goto end

:win2000
@echo           当前系统是：win2000
@echo ----------------------------------------
goto end

:win_xp
@echo           当前系统是：WinXP
@echo ----------------------------------------
goto end

:win2003
@echo           当前系统是：win200
@echo ----------------------------------------
goto end

:vista
@echo           当前系统是：vista
@echo ----------------------------------------
goto end

:win7
@echo           当前系统是：win7
@echo ----------------------------------------
xcopy "%~sdp0..\driver_win7_32\OemWin2k.inf" "%~sdp0..driver\OemWin2k.inf" /y
xcopy "%~sdp0..\driver_win7_32\tap0901.cat" "%~sdp0..\driver\tap0901.cat" /y
xcopy "%~sdp0..\driver_win7_32\tap0901.sys" "%~sdp0..\driver\tap0901.sys" /y
xcopy "%~sdp0..\driver_win7_32\tapinstall.exe" "%~sdp0tapinstall.exe" /y
goto end

:win8
@echo           当前系统是：win8
@echo ----------------------------------------
goto end

@echo           不能获取当前操作系统版本
@echo ----------------------------------------

:end
