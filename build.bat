@echo off

:: the path to the codebase
set code_dir=%cd%

:: Debug=0, Release=1
set release_mode=1

if not exist bld mkdir bld
pushd bld

del *.ilk > NUL 2> NUL
:: del *.pdb > NUL 2> NUL

set mode=
if %release_mode% EQU 1 ( 
	set mode=release
)

if not exist "%tree-sitter.lib" (
	call %code_dir%\build_tree_sitter.bat
)

call %code_dir%\custom\bin\tree_sitter_buildsuper_x64-win.bat %code_dir%\code\4coder_tree_sitter.cpp mode %cd%\tree-sitter.lib
del *.obj > NUL 2> NUL

echo copying files
xcopy custom_4coder.* ..\4ed\ /Y

popd

