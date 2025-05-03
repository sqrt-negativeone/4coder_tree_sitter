@echo off

setlocal enabledelayedexpansion

set tree_sitter_root=%code_dir%\code\tree_sitter\
set include_paths=-I%tree_sitter_root%include\ -I%tree_sitter_root%..\

set langs_root_dir=%code_dir%\langs


:: Build tree-sitter.lib

del *.obj

call cl -c /O2 %include_paths% /nologo /Zi %tree_sitter_root%src\lib.c /Fotree-sitter-lib.obj

:: NOTE: add parsers and scanners for languages here

echo building parsers

for /d %%D in ("%langs_root_dir%\*") do (
	set "dir_name=%%~nxD"
	echo Building parser for !dir_name!:
	
	for %%F in (%%D\*.*) do (
		set "outname=tree-sitter-!dir_name!-%%~nxF.obj"
    call cl -c /O2 %include_paths% /nologo /Zi %%F /Fo!outname!
	)
)


call lib /nologo *.obj /OUT:tree-sitter.lib

