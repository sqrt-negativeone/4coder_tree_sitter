

(preproc_function_def name: (identifier) @macro_def)
(preproc_def name: (identifier) @macro_def)

(type_definition
declarator: (function_declarator declarator: (type_identifier) @typedef.function)
)

 ;(function_declarator  (parenthesized_declarator (pointer_declarator [(identifier) (field_identifier)] @function_pointer)))
(function_declarator declarator: (identifier) @function_def)
(function_definition (parenthesized_declarator (identifier) @function_def))

(struct_specifier
name: (type_identifier) @typedef.struct)

(enum_specifier
name: (type_identifier) @typedef.enum)

(union_specifier
name: (type_identifier) @typedef.union)

; (type_identifier) @typedef.type

(enumerator name: (identifier) @enum.const)

(type_definition
	declarator: (parenthesized_declarator
    	(type_identifier) @typedef.type))
        
; declared types
(_declarator declarator: (type_identifier) @typedef.type)

; capture function paramaters
(function_declarator
parameters: (parameter_list
	(parameter_declaration
    	declarator: (identifier) @func_param_name)
    )+
)

