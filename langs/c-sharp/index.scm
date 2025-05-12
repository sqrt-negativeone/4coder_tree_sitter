

(method_declaration name: (identifier) @function_def)
(local_function_statement name: (identifier) @function_def)

(interface_declaration name: (identifier) @typedef.interface)
(class_declaration name: (identifier) @typedef.class)
(enum_declaration name: (identifier) @typedef.enum)
(struct_declaration (identifier) @typedef.struct)
(record_declaration (identifier) @typedef.record)

(namespace_declaration name: (identifier) @namespace_def)

(generic_name (identifier) @typedef.type)
(type_parameter (identifier) @typedef.type)
(parameter type: (identifier) @tyepdef.type)
(type_argument_list (identifier) @typedef.type)
(as_expression right: (identifier) @typedef.type)
(is_expression right: (identifier) @typedef.type)

(constructor_declaration name: (identifier) @constructor_def)
(destructor_declaration name: (identifier) @constructor_def)

(base_list (identifier) @typedef.type)
(_ type: (identifier) @typedef.type)

(enum_member_declaration (identifier) @enum.const)
