::Specify the your's project's location & name
::Use backslashes cause Windows
set prjdir=C:\dev\FlappyBird
set prjname=FlappyBird
::Specify th export location
set location=D:\Exports\Web

::Directory where Engine is located
set greenteadir=D:\GreenTea

::Compile Box2D
mkdir ..\bin\Web\Box2D
mkdir ..\bin-int\Web\Box2D
cd ..\3rdParty\box2d\src
@echo off
set objs=
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\..\..\bin-int\Web\Box2D
call em++ -c -fPIC -O2 -I../../../3rdParty/box2d/include -DNDEBUG %objs%
@echo off
set objs=
for /R %%f in (*.o) do call set objs=%%objs%% %%f
@echo on
call emar ru libBox2D.a %objs%
move libBox2D.a ..\..\..\bin\Web\Box2D
::Compile yaml-cpp../../../GreenTea/src
mkdir ..\yaml-cpp
mkdir ..\..\..\bin\Web\yaml-cpp
cd ..\..\..\3rdParty\yaml-cpp\src
@echo off
set objs=
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\..\..\bin-int\Web\yaml-cpp
call em++ -c -fPIC -O2 -I../../../3rdParty/yaml-cpp/include %objs%
@echo off
set objs=
for /R %%f in (*.o) do call set objs=%%objs%% %%f
@echo on
call emar ru libyaml-cpp.a %objs%
move libyaml-cpp.a ..\..\..\bin\Web\yaml-cpp
::Compile Engine
mkdir ..\GreenTea
mkdir ..\..\..\bin\Web\GreenTea
cd ..\..\..\GreenTea\src\Engine
@echo off
set objs=
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\Platforms\GLFW
@echo off
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\Linux
@echo off
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\OpenGL
@echo off
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
cd ..\..\..\..\bin-int\Web\GreenTea
call em++ -c -fPIC -O2 -Wno-nonportable-include-path -I../../../GreenTea/src -I../../../3rdParty/entt/single_include/entt -I../../../3rdParty/yaml-cpp/include -I../../../3rdParty/glm/glm -I../../../3rdParty/stb -I../../../3rdParty/box2d/include -DGT_DIST -DGT_WEB %objs%
@echo off
set objs=
for /R %%f in (*.o) do call set objs=%%objs%% %%f
@echo on
call em++ -s SIDE_MODULE=1 -o libGreenTea.so -fPIC ../../../bin/Web/Box2D/libBox2D.a ../../../bin/Web/yaml-cpp/libyaml-cpp.a %objs%
move libGreenTea.so ..\..\..\bin\Web\GreenTea
::::Compile StandAlone app
cd ..
mkdir StandAlone
mkdir ..\..\bin\Web\StandAlone
cd StandAlone
call em++ -c -fPIC -O2 -DGT_DIST -DGT_WEB -Wno-nonportable-include-path -I../../../GreenTea/src -I ../../../StandAlone/src -I../../../3rdParty/entt/single_include/entt -I../../../3rdParty/yaml-cpp/include -I../../../3rdParty/glm/glm^
 ../../../StandAlone/src/StandAlone.cpp ../../../StandAlone/src/main.cpp
::Build Project
cd /d %prjdir%/%prjname%
@echo off
set objs=
for /R %%f in (*.cpp) do call set objs=%%objs%% %%f
@echo on
mkdir %prjdir%\bin-int\Web\%prjname%
mkdir %prjdir%\bin\Web\%prjname%
cd %prjdir%\bin-int\Web\%prjname%
call em++ -c -fPIC -O2 -DGT_DIST -DGT_WEB -Wno-nonportable-include-path -I%prjdir%/%prjname%/src -I%greenteadir%/GreenTea/src -I%greenteadir%/3rdParty/entt/single_include/entt -I%greenteadir%/3rdParty/glm/glm -I%greenteadir%/3rdParty/yaml-cpp/include %objs%
@echo off
set objs=
for /R %%f in (*.o) do call set objs=%%objs%% %%f
@echo on
call em++ -s SIDE_MODULE=1 -o lib%prjname%.so -fPIC %greenteadir%/bin/Web/GreenTea/libGreenTea.so %objs%
move lib%prjname%.so %prjdir%/bin/Web/%prjname%
::Combine Everything
mkdir %prjdir%\bin\Web\temp\Assets\Icons
mkdir %prjdir%\bin\Web\temp\GameData\.gt
xcopy %prjdir%\Assets %prjdir%\bin\Web\temp\GameData\Assets /E /H /C /I
copy %prjdir%\bin\Web\%prjname%\lib%prjname%.so %prjdir%\bin\Web\temp\GameData\.gt
xcopy %greenteadir%\Assets\Shaders %prjdir%\bin\Web\temp\Assets\Shaders /E /H /C /I
copy %greenteadir%\Assets\Icons\Logo.png %prjdir%\bin\Web\temp\Assets\Icons
copy %prjdir%\%prjname%.gt %prjdir%\bin\Web\temp\GameData
copy %greenteadir%\bin\Web\GreenTea\libGreenTea.so %prjdir%\bin\Web\temp
copy %greenteadir%\bin-int\Web\StandAlone\main.o %prjdir%\bin\Web\temp
copy %greenteadir%\bin-int\Web\StandAlone\StandAlone.o %prjdir%\bin\Web\temp
cd /d %prjdir%\bin\Web\temp
call em++ main.o StandAlone.o libGreenTea.so -o index.html -sERROR_ON_UNDEFINED_SYMBOLS=0 -s USE_GLFW=3 -s FULL_ES3=1 --preload-file Assets --preload-file GameData -sALLOW_MEMORY_GROWTH -sALLOW_TABLE_GROWTH -s MAIN_MODULE=1
move index.data %location%
move index.html %location%
move index.js %location%
move index.wasm %location%