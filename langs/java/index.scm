

(method_declaration name: (identifier) @function_def)

(interface_declaration name: (identifier) @typedef.interface)
(class_declaration name: (identifier) @typedef.prod_type)
(enum_declaration name: (identifier) @typedef.prod_type)

(package_declaration (scoped_identifier (identifier) @package_def))
(package_declaration (identifier) @package_def)

