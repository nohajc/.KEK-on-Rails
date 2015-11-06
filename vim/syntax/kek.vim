" Vim syntax file
" Language:	.KEK

" Read the CPP syntax to start with
runtime! syntax/cpp.vim
unlet b:current_syntax

" .KEK extensions
syn keyword kekIdentifier	var
syn keyword kekTypedef		super this
syn keyword kekStatement	read write

" Default highlighting

hi def link kekIdentifier	Identifier
hi def link kekTypedef		Typedef
hi def link kekStatement	Statement

let b:current_syntax = "kek"
