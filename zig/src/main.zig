// syms
// (c) Reuben Thomas <rrt@sc3d.org>

const std = @import("std");

const c = @cImport({
    // config.h must be included first
    @cInclude("config.h");

    // System includes
    @cInclude("stdlib.h");
    @cInclude("locale.h");
    @cInclude("regex.h");

    // gnulib includes
    @cInclude("error.h");
    @cDefine("_error", "error");
    @cInclude("quote.h");
    @cInclude("xalloc.h");

    // Our includes
    @cInclude("cmdline.h");
    @cInclude("regex_alloc.h");
});

var args_info: c.gengetopt_args_info = undefined;

fn get_symbol(re: *c.regex_t, s: [*c]const u8) ?c.regmatch_t {
    var match = [1]c.regmatch_t{undefined};
    if (c.regexec(re, s, 1, &match[0], 0) != 0)
        return null;
    return match[0];
}

pub fn main() !void {
    _ = c.setlocale(c.LC_ALL, "");

    // Process command-line options
    if (c.cmdline_parser(@intCast(c_int, std.os.argv.len), @ptrCast([*c][*c]u8, std.os.argv), &args_info) != 0)
        std.os.exit(c.EXIT_FAILURE);

    // Compile regex
    var re = c.regex_t_alloc().?;
    const err = c.regcomp(re, args_info.symbol_arg, c.REG_EXTENDED);
    if (err != 0) {
        var errlen = c.regerror(err, re, null, 0);
        var errbuf = @ptrCast(*u8, c.xmalloc(errlen));
        _ = c.regerror(err, re, errbuf, errlen);
        c._error(c.EXIT_FAILURE, 0, "%s", errbuf);
    }

    // Process input
    var i: c_uint = 0;
    while (i <= args_info.inputs_num) : (i += 1) {
        if (i < args_info.inputs_num and (std.cstr.cmp(args_info.inputs[i], "-") != 0)) {
            if (c.freopen(args_info.inputs[i], "r", c.stdin) == null)
                c._error(c.EXIT_FAILURE, @enumToInt(std.os.errno(0)), "cannot open %s", c.quote(args_info.inputs[i]));
        }
        var len: usize = 0;
        var line: [*c]u8 = undefined;
        const stdout = std.io.getStdOut();
        while (c.getline(&line, &len, c.stdin) != -1) : (line = null) {
            var p = line;
            var match: c.regmatch_t = undefined;
            while (true) : (p += @intCast(usize, match.rm_eo)) {
                match = get_symbol(re, p) orelse break;
                try stdout.writer().print(
                    "{s}\n",
                    .{p[@intCast(usize, match.rm_so)..@intCast(usize, match.rm_eo)]},
                );
            }
            c.free(line);
        }
        _ = c.fclose(c.stdin);
        if (i < args_info.inputs_num - 1)
            _ = c.putchar('\n');
    }

    std.os.exit(c.EXIT_SUCCESS);
}
