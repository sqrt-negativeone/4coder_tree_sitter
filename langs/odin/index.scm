
; Namspaces

(package_declaration (identifier) @namespace_def)
(import_declaration alias: (identifier) @namespace_def)
(foreign_block (identifier) @namespace_def)

; Procedures
(procedure_declaration (identifier) @function_def (procedure (block)))
(procedure_declaration (identifier) @function_def (procedure (uninitialized)))

; Types
(procedure_declaration (identifier) @typedef.prod_type)
(struct_declaration (identifier) @typedef.prod_type "::")
(enum_declaration (identifier) @typedef.prod_type   "::")
(union_declaration (identifier) @typedef.sum_type   "::")

(const_declaration (identifier) @typedef.prod_type "::" [(array_type) (distinct_type) (bit_set_type) (pointer_type)])
