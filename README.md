# SO

## Grade: 18/20 ⭐️

A Server-Client App, made for our [SO](https://www.di.uminho.pt/~jno/sitedi/uc_J304N1.html) class.

Project requirements (set by the professors) are in `Requirements.pdf` (🇵🇹).

## Running the application

You may need to read the `Requirements.pdf` document to understand the usage.

#### Compiling
```bash
$ make
```

#### Running the server
```bash
$ ./bin/orchestrator tmp 3 fcfs
```

#### Spawning a client task
```bash
$ ./bin/client execute -u 10 "sleep 10" 
```

#### Spawning a client status
```bash
$ ./bin/client status 
```

## Contributing

As a university group project, we cannot allow external contributors.

## Group Members

* [Afonso Dionísio Santos](https://github.com/afonso-santos/) (a104276)
* [Pedro Figueiredo Pereira](https://github.com/pedrofp4444) (a104082)
* [Hélder Ricardo Ribeiro Gomes](https://github.com/helderrrg) (a104100)
