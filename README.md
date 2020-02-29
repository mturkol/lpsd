# LPSD Algorithm

> lpsd - a program to calculate spectral estimates by Michael Troebs  and
> Gerhard Heinzel, (c) 2003, 2004. Revisions and bug-fixes by Mert Türkol, 2019, 2020.

This repository provides the implementation of the LPSD algorithm for spectrum
and spectral density estimation from long time series by Fourier transforms at
frequencies equally spaced on a logarithmic axis.

It is written in C by Michael Tröbs and Gerhard Heinzel. 
Improvements on accuracy, bug-fixes and corrections in algorithm flow were  made 
by [Mert Türkol](mailto:mturkol_at_gmail_dot_com).


## Table of Contents

<!-- vim-markdown-toc GFM -->

* [Install](#install)
    * [Requirements](#requirements)
    * [Compilation](#compilation)
* [Usage](#usage)
    * [Example](#example)
    * [Options](#options)
* [Documentation](#documentation)
    * [Theoretical Background](#theoretical-background)
    * [File Overview](#file-overview)
* [FAQ](#faq)

<!-- vim-markdown-toc -->

## Install

### Requirements

Make sure you have `fftw3` installed.

### Compilation

Make use of the `Makefile` to compile the `lpsd-exec` by typing:

```
$ make
```

## Usage

`lpsd` can be controlled by command line options or interactively. 

```
$ ./lpsd-exec [OPTION...] 
```

Run `lpsd-exec` with the `help` option for a good overview of lpsd's
functionality and options:

```
$ ./lpsd-exec --help
```

### Example

Create a test dataset with:

```
$ make example-data
```

This will create the file `test.dat` in the directory `example` which can be
processed with `lpsd`:

```
$ ./lpsd-exec --input=example/test.dat
```

### Options

The command options `lpsd` understands:

| Short | Long                     | Description                                     |
| :---: | :----------------------- | :---------------------------------------------- |
| `-a`  | `--davg=# of des. avgs`  | desired number of averages                      |
| `-A`  | `--colA=# of column   `  | number of column to process                     |
| `-b`  | `--tmin=tmin          `  | start time in seconds                           |
| `-B`  | `--colB=# of column    ` | process column B - column A                     |
| `-c`  | `--param=param         ` | parameter string                                |
| `-d`  | `--usedefs             ` | use defaults                                    |
| `-e`  | `--tmax=tmax           ` | stop time in seconds                            |
| `-f`  | `--fsamp=sampl. freq.  ` | sampling frequency in Hertz                     |
| `-g`  | `--gnuplot=gnuplot file` | gnuplot file name                               |
| `-h`  | `--method=0, 1         ` | method for frequency calculation: 0-LPSD, 1-FFT |
| `-i`  | `--input=input file   `  | input file name                                 |
| `-j`  | `--fres=FFT freq. res.`  | Frequency resolution for FFT                    |
| `-k`  | `--sbin=sbin         `   | smallest frequency bin                          |
| `-l`  | `--ovlp=overlap       `  | segment overlap in %                            |
| `-m`  | `--mavg=# of min. avgs ` | minimum number of averages                      |
| `-n`  | `--nspec=# in spectr.`   | number of values in spectrum                    |
| `-o`  | `--output=output file `  | output file name                                |
| `-p`  | `--psll=psll         `   | peak side lobe level in dB                      |
| `-q`  | `--quiet          `      | Don't produce output on screen                  |
| `-r`  | `--lr=0,1         `      | linear regression; 1 yes, 0 no                  |
| `-s`  | `--fmin=fmin     `       | start frequency in spectrum                     |
| `-t`  | `--fmax=fmax      `      | stop frequency in spectrum                      |
| `-T`  | `--time          `       | file contains time in s in first column         |
| `-u`  | `--gnuterm=gnuterm  `    | number of gnuplot terminal                      |
| `-w`  | `--window=wind. func.`   | window function; -2 Kaiser, -1 flat top, 0..30  |
| `-x`  | `--scale=factor    `     | scaling factor                                  |
| `-?`  | `--help           `      | Give this help list                             |
| `-V`  | `--version    `          | Print program version                           |

## Documentation

### Theoretical Background

For a more theoretical understanding read the following publication:

  * [https://doi.org/10.1016/j.measurement.2005.10.010](https://doi.org/10.1016/j.measurement.2005.10.010)

### File Overview

| File          | Description                              |
| ------------- | ---------------------------------------- |
| `ArgParser.c` | Parses the given arguments               |
| `ask.c`       | Manages user input in interactive mode   |
| `calibrate.c` |                                          |
| `CHANGELOG`   | Changelog                                |
| `config.c`    | Configure lpsd at runtime via a textfile |
| `debug.c`     | Debugging                                |
| `errors.c`    | error messages                           |
| `genwin.c`    | compute window functions                 |
| `goodn.c`     |                                          |
| `IO.c`        | handle all input/output for `lpsd.c`     |
| `libargp.a`   | static library for argument parsing      |
| `lpsd`        | Executable                               |
| `lpsd.c`      |                                          |
| `lpsd.cfg`    | Configuration file                       |
| `lpsd-exec.c` |                                          |
| `Makefile`    | To build the executable                  |
| `misc.c`      |                                          |
| `netlibi0.c`  |                                          |
| `README.md`   | This README.md                           |
| `StrParser.c` |                                          |
| `tics.c`      |                                          |

## FAQ

* Hot can I enable `lpsd` configuration via config file?

To enable configuration of lpsd via `lpsd.cfg`, do one of the following under
linux, use the command

```
$ export LPSDCFN=/your/dir/lpsdconfigfilename
```

**Example:**

```
$ export LPSDCFN=/home/micha/lpsd.cfg
```

under DOS, use the command

```
$ set LPSDNCFN=c:\your\dir\lpsdconfigfilename
```

Example:

```
$ set LPSDCFN=c:\tools\lpsd.cfg
```
