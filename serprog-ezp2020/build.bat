@echo off
setlocal

:: --------------------------------------------------------------------------
:: build.bat - Build ezp2020-serprog firmware using bundled tools
:: --------------------------------------------------------------------------
set TOOLSDIR=..\Tools\build\bin
set SDCC=%TOOLSDIR%\sdcc.exe
set OBJCOPY=%TOOLSDIR%\avr-objcopy_7_3_0.exe
set TARGET=ezp2020-serprog

:: SDCC flags for CH552T (8051 core, 24 MHz)
::   --xram-size 0x0300  : XRAM available for user code (0x0100..0x03FF)
::   --xram-loc  0x0100  : XRAM user region starts at 0x0100
::                         (0x0000..0x00FF is reserved for USB DMA buffers in
::                          Ep0Buffer / Ep1Buffer / Ep2Buffer / Ep2Buffer_1)
::   --code-size 0x3800  : 14 KB flash (leaves room for bootloader)
set CFLAGS=-mmcs51 --model-small --xram-size 0x0300 --xram-loc 0x0100 --code-size 0x3800 -DFREQ_SYS=24000000 -Iinclude
set OUTDIR=build

if not exist %OUTDIR% mkdir %OUTDIR%

echo [1/4] Compiling debug.c ...
"%SDCC%" %CFLAGS% -c include\debug.c -o %OUTDIR%\debug.rel
if errorlevel 1 goto :err

echo [2/4] Compiling spi.c ...
"%SDCC%" %CFLAGS% -c include\spi.c -o %OUTDIR%\spi.rel
if errorlevel 1 goto :err

echo [3/4] Compiling main.c ...
"%SDCC%" %CFLAGS% -c main.c -o %OUTDIR%\main.rel
if errorlevel 1 goto :err

echo [4/4] Linking ...
"%SDCC%" %CFLAGS% %OUTDIR%\debug.rel %OUTDIR%\spi.rel %OUTDIR%\main.rel -o %OUTDIR%\%TARGET%.ihx
if errorlevel 1 goto :err

echo Converting to binary ...
"%OBJCOPY%" -I ihex -O binary %OUTDIR%\%TARGET%.ihx %TARGET%.bin
if errorlevel 1 goto :err

echo.
echo Done! Flash file: %TARGET%.bin
echo.
echo Flash with wchisp (put CH552 in ISP mode first):
echo   wchisp flash %TARGET%.bin
goto :end

:err
echo.
echo BUILD FAILED.
exit /b 1

:end
endlocal
