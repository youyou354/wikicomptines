# Wikicomptines — a client/server in C over TCP

L2 double degree in mathematics and computer science (Université Sorbonne Paris Nord, Institut Galilée),
course *Système et réseaux*, spring 2024.

A server that holds a catalogue of nursery rhymes (*comptines*) and serves them over TCP with a small
application protocol, **WCP — Wikicomptine Protocol**, and a terminal client that talks to it.

## The protocol (port 4321)

1. The client connects; the server sends the catalogue: one line per rhyme, its number written in decimal on
   6 characters, a space, then its title — and an empty line to finish.
2. The client sends the number it wants, as a 2-byte integer in network byte order.
3. The server sends that rhyme, line by line, and two empty lines to finish. The connection closes.

The server is **multithreaded**: every accepted connection gets its own `pthread` with a small
`struct thread` (the socket, the rhymes directory, a pointer to the shared catalogue), and every exchange is
logged in `log_clients.txt` with the client's IP address.

## Build and run

```sh
make
./wcp_srv comptines        # in one terminal
./wcp_clt 127.0.0.1        # in another — or several at once
```

| file | what it does |
|------|--------------|
| `wcp_srv.c` | listening socket, `accept` loop, one thread per client, catalogue, sending the list and a rhyme, log |
| `wcp_clt.c` | connects, prints the list, asks for a valid number (and survives letters typed instead), receives the rhyme |
| `comptine_utils.c/.h` | reading a line up to `\n`, recognising `.cpt` files, building the catalogue from a directory |
| `rapport.pdf` | my report: the problems met and how they were solved |

The skeleton of the files and the rhymes in `comptines/` were provided by the course; the code is mine.

On my portfolio, an animated version of a WCP session runs in the browser: <https://youyou354.github.io/>.
