
; Namspaces

(package_declaration (identifier) @namespace_def)
(import_declaration alias: (identifier) @namespace_def)
(foreign_block (identifier) @namespace_def)

; Procedures
(procedure_declaration (identifier) @function_def (procedure (block)))
(procedure_declaration (identifier) @function_def (procedure (uninitialized)))

; Types
(procedure_declaration (identifier) @typedef.type)
(struct_declaration (identifier) @typedef.struct "::")
(enum_declaration (identifier) @typedef.enum "::")
(union_declaration (identifier) @typedef.union "::")

(const_declaration (identifier) @typedef.type "::" [(array_type) (distinct_type) (bit_set_type) (pointer_type)])
