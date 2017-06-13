@echo off
setlocal ENABLEEXTENSIONS

if "%1" == "" goto PROMPT
if "%2" == "" goto PROMPT
set LIB_LIST=(app.lib baseevent.lib log.lib boost.lib sdt.lib stn.lib openssl.lib)
set COMM_LIB=comm.lib
set DST_LIB= %1
set LIB_PATH= %2

if not exist "%LIB_PATH%/%COMM_LIB%" (
	@echo !!BUILD FAILED!!, "%LIB_PATH%/%COMM_LIB%" not exist
	exit /B 1
)

DEL /F/Q %DST_LIB%
LIB.EXE /out:%DST_LIB% %LIB_PATH%/%COMM_LIB%
for %%i in %LIB_LIST% do (
	if exist "%LIB_PATH%/%%i" (
		LIB.exe %DST_LIB% "%LIB_PATH%/%%i"
		if %ERRORLEVEL% NEQ 0 (
			@echo !!BUILD FAILED!!, LIB.exe execute failed. 
			exit /B 2
		)		
	)else (
		@echo LIB "!!WARNING!! %LIB_PATH%/%%i" not exist...
	)
)

@echo BUILD LIB %DST_LIB% COMPLETED.
EXIT /B 0
:PROMPT
@echo Usage: %0 dst-lib-filename libs-directory
