This compiler snapshot provides a small native runtime surface in generated C++:
- `stdlib::io`: `printn(value)` and `printfn(values...)` output helpers.
- `stdlib::math`: `sqrt(value)`, `pow(base, exponent)`, `absolute(value)`.
- `stdlib::str`: `length(value)`, `parse_i32(string)`, `parse_f64(string)`.

These helpers are currently compiler-runtime built-ins rather than independently compiled `.gill` modules.

- `stdlib::io`: `printf(format, arguments...)` and `printfn(format, arguments...)` use format identifiers. Supported identifiers in this snapshot are `%d`, `%u`, `%f`, `%c`, `%s`, `%b`, `%x`, `%o`, and `%%`; basic width and precision are accepted.
- Format argument count and type are checked by the generated runtime helper. Compile-time diagnostics for format strings are not implemented yet.
- `main` accepts no parameters, `(i32 argc, string[] argv)`, or `(i32 argc, zString* argv)` in this compiler snapshot.
