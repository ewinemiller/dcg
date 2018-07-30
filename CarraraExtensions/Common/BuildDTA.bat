rem Compile the DTA file using Apple Computer Rez tool
rem Usage _ BuildDTA Debug_Release ExtensionName
mkdir "%1"
echo Calling Rez...
call ..\..\QT_5.0.1_Win_SDK\QT501SDK\SDK\Tools\Rez.exe -I "..\..\CarraraExtensions\Common" -I ..\..\QT_5.0.1_Win_SDK\QT501SDK\SDK\CIncludes -I ..\..\QT_5.0.1_Win_SDK\QT501SDK\SDK\RIncludes -I ..\..\%3\Include\Common\ -I ..\..\%3\Include\Carrara\ -d "WIN=1" -o %1\%2 %2.r
echo ...back from Rez
if exist %1\%2.dat del %1\%2.dat
if exist %1\%2.txt del %1\%2.txt
..\..\%3\Tools\ResExtracter\Win\ResExtracter.exe %1\%2.qtr
copy %1\%2.dat %4\%2.dat
copy %1\%2.txt %4\%2.txt
goto END
:END