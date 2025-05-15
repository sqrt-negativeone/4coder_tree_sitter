(namespace_definition (namespace_identifier) @namespace_def)

(preproc_function_def name: (identifier) @macro_def)
(preproc_def name: (identifier) @macro_def)

(type_definition
declarator: (function_declarator declarator: (type_identifier) @typedef.prod_type)
)

 ;(function_declarator  (parenthesized_declarator (pointer_declarator [(identifier) (field_identifier)] @function_pointer)))
(function_declarator declarator: (identifier) @function_def)
(function_definition (parenthesized_declarator (identifier) @function_def))
(function_declarator declarator: (field_identifier) @name) @function_def

(class_specifier name: (type_identifier) @name) @typedef.prod_type

(struct_specifier
name: (type_identifier) @typedef.prod_type)

(enum_specifier
name: (type_identifier) @typedef.prod_type)

(union_specifier
name: (type_identifier) @typedef.sum_type)

; (type_identifier) @typedef.prod_type

(enumerator name: (identifier) @constant)

(type_definition
	declarator: (parenthesized_declarator
    	(type_identifier) @typedef.prod_type))
        
; declared types
(_declarator declarator: (type_identifier) @typedef.prod_type)

; capture function paramaters
(function_declarator
parameters: (parameter_list
	(parameter_declaration
    	declarator: (identifier) @func_param_name)
    )+
)

