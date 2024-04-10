# Macaulay
ANSI conversion for 1994 Macaulay code

This project is finished, unless we discover conversion bugs or want to have `convert.rb` handle further automatic changes.

Macaulay still crashes; when we get it running we'll update `PatchFile`
to output a working 2022 release.

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
