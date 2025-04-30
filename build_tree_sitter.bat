@echo off

set tree_sitter_root=%code_dir%\code\tree_sitter\
set include_paths=-I%tree_sitter_root%include\

:: Build tree-sitter.lib

del *.obj

call cl -c /O2 %include_paths% /nologo /Zi %tree_sitter_root%src\lib.c /Fotree-sitter-lib.obj

:: NOTE: add parsers and scanners for languages here

:: C parser
call cl -c /O2 %include_paths% /nologo /Zi %tree_sitter_root%..\langs\c\parser.c /Fotree-sitter-c.obj


call lib /nologo *.obj /OUT:tree-sitter.lib


