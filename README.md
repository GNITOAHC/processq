# Processq

A simple binary that can submit process to local machine's background.

## Installation

### Pre-built binaries

Pre-built binaries are available on the [releases page](https://github.com/GNITOAHC/processq/releases)

### Homebrew

```bash
brew tap gnitoahc/tap
brew install gnitoahc/tap/queue
```

## Usage

```
Usage: queue [OPTIONS]
Options:
  -h, --help                    Display this help message
  -v, --version                 Display version information
  -c, --config [FILE]           Specify a configuration file (WIP)
  -o, --out [DIR]               Specify a output directory
  -m, --cmd [COMMAND]           Specify a command to run
  -l, --list                    List all running processes
```

## Test

```
make && ./bin/main --cmd "bash test.sh"
```
