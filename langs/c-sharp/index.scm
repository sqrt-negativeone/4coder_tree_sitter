

(method_declaration name: (identifier) @function_def)
(local_function_statement name: (identifier) @function_def)

(interface_declaration name: (identifier) @typedef.interface)
(class_declaration name: (identifier) @typedef.prod_type)
(enum_declaration name: (identifier) @typedef.prod_type)
(struct_declaration (identifier) @typedef.prod_type)
(record_declaration (identifier) @typedef.prod_type)

(namespace_declaration name: (identifier) @namespace_def)

(generic_name (identifier) @typedef.prod_type)
(type_parameter (identifier) @typedef.prod_type)
(parameter type: (identifier) @tyepdef.prod_type)
(type_argument_list (identifier) @typedef.prod_type)
(as_expression right: (identifier) @typedef.prod_type)
(is_expression right: (identifier) @typedef.prod_type)

(constructor_declaration name: (identifier) @constructor_def)
(destructor_declaration name: (identifier) @constructor_def)

(base_list (identifier) @typedef.prod_type)
(_ type: (identifier) @typedef.prod_type)

(enum_member_declaration (identifier) @constant)
