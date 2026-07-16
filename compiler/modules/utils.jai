#import "Basic";

// Set by error_assert() in library mode (see below) instead of exiting, so
// callers can unwind explicitly with `if compile_failed return ...;` using
// the right zero-value for their own return type -- #expand's `return` can't
// infer that generically across callers with different signatures.
compile_failed: bool;
compile_error: string;

error_assert :: (cond: bool, message: string) {
    if ! cond {
        compile_failed = true;
        compile_error = message;
        log("Compilation error: %\n", message);
    }
}

#if #exists(global_args) {
    verbose_print :: (format_string: string, args: .. Any) {
        if (global_args.verbose) {
            log(format_string, ..args);
        }
    }
} else {
    verbose_print :: (format_string: string, args: .. Any) {
        // print(format_string, args); // @TODO??
    }
}
