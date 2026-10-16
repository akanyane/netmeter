# netmeter

Ferramenta de linha de comando, escrita em C99 com libcurl, que mede a velocidade da sua internet usando os endpoints públicos de teste da Cloudflare (`speed.cloudflare.com`).

## O que ele mede

| Teste | Como funciona | Resultado |
|-------|---------------|-----------|
| **Latência (ping)** | Abre 5 conexões novas e mede o handshake TCP (`CURLINFO_CONNECT_TIME_T` menos `CURLINFO_NAMELOOKUP_TIME_T`) | Mínimo, média e jitter em ms |
| **Download** | Baixa `https://speed.cloudflare.com/__down?bytes=N` descartando o conteúdo | Velocidade em Mbps |
| **Upload** | Envia um buffer via POST para `https://speed.cloudflare.com/__up` | Velocidade em Mbps |

- O **jitter** é a média da diferença absoluta entre amostras consecutivas de latência.
- Em download e upload, o tempo de DNS, TCP e TLS é descontado (`CURLINFO_PRETRANSFER_TIME_T`), então a velocidade reflete apenas a transferência dos dados.
- Durante os testes, uma barra de progresso é exibida no `stderr` (via `CURLOPT_XFERINFOFUNCTION`). Ela só aparece em terminal interativo e é desligada com `--json`.
- Os tamanhos usam MB = 1024 × 1024 bytes; o resultado em Mbps usa megabits decimais (10⁶ bits/s).

## Instalando a libcurl

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential libcurl4-openssl-dev pkg-config
```

### Windows (MSYS2, ambiente UCRT64)

Abra o terminal **MSYS2 UCRT64** e execute:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
                   mingw-w64-ucrt-x86_64-make \
                   mingw-w64-ucrt-x86_64-curl \
                   mingw-w64-ucrt-x86_64-pkgconf
```

## Compilando

```bash
make            # gera o executável (netmeter, ou netmeter.exe no Windows)
make clean      # remove build/ e o executável
```

No MSYS2 UCRT64, use `mingw32-make` se o `make` não estiver no PATH. O projeto compila com `-std=c99 -Wall -Wextra` sem avisos.

### Instalando o binário

```bash
sudo make install                      # instala em /usr/local/bin
make install PREFIX=$HOME/.local       # instala só para o seu usuário
sudo make uninstall                    # remove
```

## Usando

```bash
./netmeter                # ping + download (25 MB) + upload (10 MB)
./netmeter -p             # somente latência
./netmeter -d -s 100      # somente download com 100 MB
./netmeter -u             # somente upload
./netmeter -j             # saída em JSON
make run ARGS="-s 5"      # compila (se preciso) e executa com argumentos
```

### Opções

| Opção | Descrição |
|-------|-----------|
| `-d`, `--download-only` | Executa somente o teste de download |
| `-u`, `--upload-only` | Executa somente o teste de upload |
| `-p`, `--ping-only` | Executa somente o teste de latência |
| `-s`, `--size <MB>` | Tamanho dos testes em MB, de 1 a 1024 (padrão: 25 MB download, 10 MB upload) |
| `-j`, `--json` | Saída em formato JSON |
| `-h`, `--help` | Mostra a ajuda |

`-d`, `-u` e `-p` são mutuamente exclusivas. Em caso de erro (sem rede, timeout, falha de TLS etc.), o programa imprime uma mensagem em português no `stderr` e termina com código de saída 1.

## Exemplo de saída

```text
$ ./netmeter
Medindo latência...
=== netmeter ===
Latência (5 conexões)
  Mínimo :    12.82 ms
  Média  :    14.21 ms
  Jitter :     2.44 ms
Download :   507.97 Mbps  (25.0 MB em 0.41 s)
Upload   :    32.50 Mbps  (10.0 MB em 2.58 s)
```

Com `--json`:

```json
{"ping":{"min_ms":6.65,"avg_ms":10.25,"jitter_ms":3.91,"samples":5},"download":{"mbps":92.16,"bytes":5242880,"seconds":0.455},"upload":{"mbps":97.83,"bytes":5242880,"seconds":0.429}}
```

> Os valores variam a cada execução, conforme a rede e a distância até o servidor da Cloudflare.

## Estrutura

```text
netmeter/
├── Makefile
├── include/   # download.h, ping.h, upload.h, utils.h
└── src/       # main.c, download.c, ping.c, upload.c, utils.c
```

## Licença

Domínio público, sob a [Unlicense](LICENSE).
