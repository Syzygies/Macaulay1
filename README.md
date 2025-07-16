# Macaulay
ANSI conversion for 1994 Macaulay code

This project stalled in 2022. It compiled and crashed. We've returned to it in 2025 with the help of Claude Code Opus 4. The issues Claude has discovered and fixed make us realize we never stood a chance before, over three decades since touching this code. There's a reason there are no automatic K&R C to C23 translation tools: K&R C is badly underspecified, and any successful modernization effort takes serious thought. The original source only ran by luck on 32 bits, and now we need code that works on 64 bits.

We've found various runtime bugs, and determined that a better workflow is to satisfy every compiler tool we can, however pedantic, to perfect the code.

We hope to use [PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) to continue this effort.

We are the original authors. This is not a fork: Git wasn't started until 2005 and this project was under development through 1994. We gave people floppy disks, wrote tape for them, and migrated to a "tar" archive when that became possible.

- `bin/` : conversion commands
  - `convert.rb` : converts 1994 K&R C Macaulay source files to ANSI C
  - `compare.sh` : backup comparison folders to `compare/`
  - `patchfile.sh` : record new PatchFile from `work/stage` to `source`
  - `today.sh` : create today's notes directory
- `build/` : binary and dependency files for build  (not archived)
- `compare/` : directory of backup comparison folders created by `compare.sh`
- `data/` : data and log files for `convert.rb`
  - `PatchFile` : diffs used to create `source/` from `work/stage/`
  - `declarations.dump` : hash saved by `convert.rb`
  - `log.txt` : verbose logging of function definition processing
  - `log.c` : sorted listing of function declarations
- `errors/` : output of logging version of `build.ninja` (not archived)
- `notes/` : dated notes directories
- `source/` : converted Macaulay source code
- `tar/` : original Macaulay sources from 1994
  - `Msource.tar` : source files used for conversion
- `work/` : intermediate source folders for conversion
  - `raw/` : original sources processed by `unifdef` and `astyle`
  - `stage/` : `convert.rb` output, before applying `PatchFile`
- `Mike/` : updates from Mike
- `build.ninja` : build file for `ninja` command
- `configure.rb` : script to create build file
  - `configure.rb 1` development version
- `Macaulay.code-workspace` : VS Code project file 

`ninja` commands:

- `ninja build` (default) : link build/Macaulay.bin
- `ninja clean` : rm -rf build

`ninja` commands (development version):

- `ninja build` (default) : convert, build, link as needed, save errors to errors/
- `ninja convert` : run convert.rb
- `ninja clean` : rm -rf build errors
- `ninja reset` : rm -rf build errors work source
