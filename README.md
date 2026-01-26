# Processq

[![Production Release](https://github.com/GNITOAHC/processq/actions/workflows/release.yml/badge.svg)](https://github.com/GNITOAHC/processq/actions/workflows/release.yml)

Sometimes we want to run a service or a task in the background, but writing a systemd configuration or using Docker is overkill when all we need is a simple, long-running daemon. This is where `queue` steps in. It runs processes in the background and provides straightforward commands to manage them.

## Installation

### Quick Install

```bash
curl -LsSf https://raw.githubusercontent.com/GNITOAHC/processq/main/install.sh | sh
```

or using `wget`:

```bash
wget -qO- https://raw.githubusercontent.com/GNITOAHC/processq/main/install.sh | sh
```

### Homebrew

```bash
brew tap gnitoahc/tap
brew install gnitoahc/tap/queue
```

### Pre-built binaries

Pre-built binaries are also available on the [releases page](https://github.com/GNITOAHC/processq/releases).

## Usage

```
Usage: queue [flags] <command>

Flags:
  -h, --help            Show this help message
  -v, --version         Enable verbose output

Commands:
  submit, sub, s     Submit a task
  list, l, ls        List all running processes
  stop, t, x         Stop a running process
  restart, r         Restart a running process
```

## Test

```
sh compile.sh && ./bin/main submit "bash test.sh"
```
