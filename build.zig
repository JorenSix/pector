const std = @import("std");

pub fn build(b: *std.build.Builder) void {

    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    if (target.getCpuArch() == .wasm32) {
        const cflagslib = [_][]const u8{ "-Wall", "-Wextra", "-Werror=return-type", "-std=gnu11", "-O2", "-fPIC" };
        const lib = b.addStaticLibrary("pector", null);

        //lib.addCSourceFile("src/pector.c", &cflagslib);
        lib.addCSourceFile("src/pector_reader.c", &cflagslib);
        lib.addCSourceFile("src/pector_config.c", &cflagslib);
        lib.addCSourceFile("src/pector_stream_processor.c", &cflagslib);
        lib.addCSourceFile("src/pector_util.c", &cflagslib);
        lib.addCSourceFile("src/pffft.c", &cflagslib);
        lib.linkLibC();

        lib.setTarget(target);
        lib.setBuildMode(mode);
        lib.install();
    } else {
        const cflags = [_][]const u8{ "-Wall", "-Wextra", "-Werror=return-type", "-std=gnu11", "-O2" };

        const exe = b.addExecutable("pector_c", null);

        exe.addCSourceFile("src/pector.c", &cflags);
        exe.addCSourceFile("src/pector_reader.c", &cflags);
        exe.addCSourceFile("src/pector_config.c", &cflags);
        exe.addCSourceFile("src/pector_stream_processor.c", &cflags);
        exe.addCSourceFile("src/pector_util.c", &cflags);
        exe.addCSourceFile("src/pffft.c", &cflags);
        exe.linkLibC();

        exe.setTarget(target);
        exe.setBuildMode(mode);
        exe.install();
    }
}
