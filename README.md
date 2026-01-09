# dedup - CLI command and library to dedup files from a directory

<a id="TOC"></a>
## Table of contents

* [Introduction](#introduction)
* [Installation](#installation)
* [Quick start](#quick-start)
* [Usage](#usage)
* [Example](#example)
* [Library](#library)
* [Known issues and limitations](#known-issues-and-limitations)
* [Getting help](#getting-help)
* [License](#license)

<a id="introduction"></a>
## Introduction

@b dedup is a C language library to deduplicate all indentical files, by hash value, found
in a working directory, when compared to a different reference directory.

Also an accompanying CLI command named `dedup` is built on top of the library.

#### In short, the dedup process works like this:

* A list of files with hashes is created from the working directory.

* An AVL tree with unique HASH values of the contents of each file in a reference directory is created.

* Each file in the list is checked for a matching HASH in the reference AVL.  If a match is found, then the file will be remove if the option for removal is set, or just reported if the option for removal is not set.

[Back to Table of Contents](#TOC)

<a id="installation"></a>
## Installation

A reasonably modern version of GNU Autotools should be installed before installing @b dedup from the source files.   If you have installed @b dedup from a ready made DPKG package, then the rest of these installation instructions do not apply.  At the time of writing this document, Autotools version 2.71 were used.

To compile the source code, the standard Debian build essentials packages are required.

This package requires that the libavl and libllist development packages are installed.  These packages should be available from the provider as @ dedup itself.

If you want to create the Doxygen API documentation, then of course Doxygen and friends need to installed.

If you have obtained the @b dedup source code from a GIT repository, then execute the `autoreconf` program while in the directory that the @b dedup GIT clone is located.

For both GIT clones and GNU Package tarballs, run the usual `./configure` followed by `make`.

To install the built `dedup` utility and **libdedup** library, run `sudo make install`.

[Back to Table of Contents](#TOC)

<a id="quick-start"></a>
## Quick start

  * to see the `dedup` help screen: run `dedup -h`
  * to dedup files in <i> /path/to/working.dir </i>:  run `dedup -r /path/to/reference.dir -w /path/to/working.dir`

[Back to Table of Contents](#TOC)

<a id="usage"></a>
## Usage

    dedup [-d] [-f] [-q] -r <reference directory> -w <working directory>

      where:

        <reference directory> is root of directory to check for duplicates
        <working directory> is root of directory where duplicates will be
                                   removed

        -d -- required to actually remove duplicates, default: false

        -f -- force; don't warn user before removal, default: false

        -q -- quiet; don't display options or progress, default: false

        -h -- this help screen

      NOTE:  It is highly recommended to run dedup without the -d option first,
             then verify that all files listed for removal are OK to remove,
             then re-run dedup with the -d option

      WARNING:  Once a file is removed by dedup it will be impossible or very
                difficult to recover that file.

[Back to Table of Contents](#TOC)

<a id="example"></a>
## Example

Using the files in the **tmp.a** and **tmp.b** directories, located in the **examples** directory of the distribution:

&nbsp;&nbsp;`ls -R tmp.a tmp.b`

yields:

    tmp.a:
      Makefile
      a.dir
      b.dir
      build
      fp
      fp.c
      picks

    tmp.a/a.dir:
      a.file
      things

    tmp.a/b.dir:
      b.file
      mystuff

    tmp.b:
      Makefile
      a.dir
      b.dir
      build
      fp
      fp.c
      picks

    tmp.b/a.dir:
      a.file
      things

    tmp.b/b.dir:
      b.file
      mystuff

Assuming that the contents of **tmp.a** and **tmp.b** are identical:

&nbsp;&nbsp;`dedup -r tmp.a -w tmp.b`

produces the following output:

    reference directory : tmp.a
    working directory   : tmp.b
    remove files        : false
    force removal       : false
    quiet               : false
    Would remove tmp.b/picks
    Would remove tmp.b/a.dir/things
    Would remove tmp.b/a.dir/a.file
    Would remove tmp.b/b.dir/mystuff
    Would remove tmp.b/b.dir/b.file
    Would remove tmp.b/fp
    Would remove tmp.b/fp.c
    Would remove tmp.b/build
    Would remove tmp.b/Makefile

The above output simply indicates what files would be removed if the `-d` option had been used.

Running:

&nbsp;&nbsp;`dedup -df -r tmp.a -w tmp.b`

produces the following output:

    reference directory : tmp.a
    working directory   : tmp.b
    remove files        : true
    force removal       : true
    quiet               : false
    REMOVED tmp.b/picks
    REMOVED tmp.b/a.dir/things
    REMOVED tmp.b/a.dir/a.file
    REMOVED tmp.b/b.dir/mystuff
    REMOVED tmp.b/b.dir/b.file
    REMOVED tmp.b/fp
    REMOVED tmp.b/fp.c
    REMOVED tmp.b/build
    REMOVED tmp.b/Makefile

The removals can be verified by again running:

&nbsp;&nbsp;`ls -R tmp.a tmp.b`

yielding:

    tmp.a:
      Makefile
      a.dir
      b.dir
      build
      fp
      fp.c
      picks

    tmp.a/a.dir:
      a.file
      things

    tmp.a/b.dir:
      b.file
      mystuff

    tmp.b:
      a.dir
      b.dir

    tmp.b/a.dir:

    tmp.b/b.dir:

[Back to Table of Contents](#TOC)

<a id="library"></a>
## Library

A static library named libdedup.a should be installed with the `dedup` CLI utility.
To compile your code using libdedup.a:

&nbsp;&nbsp;Run `cc $(pkg-config --cflags --libs dedup) -o <yourprogram> <yourprogram>.c`

[Back to Table of Contents](#TOC)

<a id="known-issues-and-limitations"></a>
## Known issues and limitations

At the time of writing of this document, there are no known issues with the @b dedup software.

Currently, @b dedup can not deduplicate the file in a single directory, therefore running:

&nbsp;&nbsp;`dedup -r a.dir -w a.dir`

will produce an error.

[Back to Table of Contents](#TOC)

<a id="getting-help"></a>
## Getting help

If you encounter any bugs or require additional help, contact [Patrick Head](mailto:patrickhead@gmail.com)

[Back to Table of Contents](#TOC)

<a id="license"></a>
## License

All source code and configuration data for the dedup software is licensed under the [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.en.html) or later.   See **COPYING** for details.

_This_ README file is itself distributed under the terms of the [Creative Commons 1.0 Universal license (CC0)](https://creativecommons.org/publicdomain/zero/1.0/).

[Back to Table of Contents](#TOC)

