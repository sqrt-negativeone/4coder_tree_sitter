

(function_declaration name: (identifier) @function_def)
(method_declaration name: (field_identifier) @function_def)

(type_spec name: (type_identifier) @typedef.prod_type)
(type_declaration (type_spec name: (type_identifier) @tyepdef.interface type: (interface_type)))
(type_declaration (type_spec name: (type_identifier) @typedef.prod_type type: (struct_type)))