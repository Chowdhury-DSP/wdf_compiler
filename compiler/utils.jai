error_assert :: (cond: bool, message: string) #expand {
    if ! cond {
        print("Compilation error: %\n", message);
        exit(1);
    }
}

#if #exists(global_args ) {
    verbose_print :: (format_string: string, args: .. Any) {
        if (global_args.verbose) {
            print(format_string, ..args);
        }
    }
} else {
    verbose_print :: (format_string: string, args: .. Any) {
        // print(format_string, args); // @TODO??
    }
}
