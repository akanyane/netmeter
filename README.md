# netmeter

A command-line tool written in C99 with libcurl that measures your internet speed using Cloudflare's public test endpoints (`speed.cloudflare.com`).

## What it measures

| Test | How it works | Result |
|------|--------------|--------|
| **Latency (ping)** | Opens 5 fresh connections and measures the TCP handshake (`CURLINFO_CONNECT_TIME_T` minus `CURLINFO_NAMELOOKUP_TIME_T`) | Minimum, average and jitter in ms |
| **Download** | Downloads `https://speed.cloudflare.com/__down?bytes=N`, discarding the content | Speed in Mbps |
| **Upload** | Sends a buffer via POST to `https://speed.cloudflare.com/__up` | Speed in Mbps |

- **Jitter** is the mean absolute difference between consecutive latency samples.
- For download and upload, DNS, TCP and TLS setup time is excluded (`CURLINFO_PRETRANSFER_TIME_T`), so the speed reflects only the data transfer.
- A progress bar is drawn on `stderr` during the tests (via `CURLOPT_XFERINFOFUNCTION`). It only appears in an interactive terminal and is disabled with `--json`.
- Sizes use MB = 1024 × 1024 bytes; the Mbps result uses decimal megabits (10⁶ bits/s).

## Installing libcurl

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential libcurl4-openssl-dev pkg-config
```

### Windows (MSYS2, UCRT64 environment)

Open the **MSYS2 UCRT64** terminal and run:

```bash
pacman -S --needed make \
                   mingw-w64-ucrt-x86_64-gcc \
                   mingw-w64-ucrt-x86_64-curl \
                   mingw-w64-ucrt-x86_64-pkgconf
```

## Building

```bash
make            # builds the executable (netmeter, or netmeter.exe on Windows)
make test       # builds and runs the unit tests (no network needed)
make clean      # removes build/ and the executable
```

The project builds with `-std=c99 -Wall -Wextra` without warnings. On Linux and in MSYS2 UCRT64 `make` is enough.

### Installing the binary

```bash
sudo make install                      # installs to /usr/local/bin
make install PREFIX=$HOME/.local       # installs for your user only
sudo make uninstall                    # removes it
```

Prebuilt binaries for Linux (`.tar.gz`, needs `libcurl4`) and Windows (`.zip`, DLLs included) are attached to each [release](../../releases).

## Usage

```bash
./netmeter                # ping + download (25 MB) + upload (10 MB)
./netmeter -p             # latency only
./netmeter -d -s 100      # download only, 100 MB
./netmeter -u             # upload only
./netmeter -j             # JSON output
make run ARGS="-s 5"      # builds (if needed) and runs with arguments
```

### Options

| Option | Description |
|--------|-------------|
| `-d`, `--download-only` | Run only the download test |
| `-u`, `--upload-only` | Run only the upload test |
| `-p`, `--ping-only` | Run only the latency test |
| `-s`, `--size <MB>` | Test size in MB, from 1 to 1024 (default: 25 MB download, 10 MB upload) |
| `-j`, `--json` | Output in JSON format |
| `-h`, `--help` | Show the help |

`-d`, `-u` and `-p` are mutually exclusive. On failure (no network, timeout, TLS error, etc.) the program prints a clear message to `stderr` and exits with status 1.

## Example output

```text
$ ./netmeter
Measuring latency...
=== netmeter ===
Latency (5 connections)
  Minimum :     8.05 ms
  Average :    10.95 ms
  Jitter  :     3.30 ms
Download :   132.10 Mbps  (25.0 MB in 1.59 s)
Upload   :   235.38 Mbps  (10.0 MB in 0.36 s)
```

With `--json`:

```json
{"ping":{"min_ms":6.65,"avg_ms":10.25,"jitter_ms":3.91,"samples":5},"download":{"mbps":92.16,"bytes":5242880,"seconds":0.455},"upload":{"mbps":97.83,"bytes":5242880,"seconds":0.429}}
```

> Values vary on every run, depending on your network and the distance to the Cloudflare server.

## Project layout

```text
netmeter/
├── Makefile
├── include/   # download.h, ping.h, upload.h, utils.h
├── src/       # main.c, download.c, ping.c, upload.c, utils.c
└── tests/     # test_netmeter.c
```

## License

Public domain, under the [Unlicense](LICENSE).
