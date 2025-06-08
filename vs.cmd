@echo off
git submodule update --init
call "tools/premake5.exe" vs2022