This compiler snapshot provides a small native runtime surface in generated C++:
- `stdlib::io`: `printn(value)` and `printfn(values...)` output helpers.
- `stdlib::math`: `sqrt(value)`, `pow(base, exponent)`, `absolute(value)`.
- `stdlib::str`: `length(value)`, `parse_i32(string)`, `parse_f64(string)`.

These helpers are currently compiler-runtime built-ins rather than independently compiled `.gill` modules.
