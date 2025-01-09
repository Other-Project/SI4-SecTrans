# SecTrans

<p align=center>
  <span>Project realized by <a href="https://github.com/AlbanFALCOZ">Alban Falcoz</a>, <a href="https://github.com/06Games">Evan Galli</a>,  <a href="https://github.com/Alexandre-Gripari">Alexandre Gripari</a> and <a href="https://github.com/theoLassauniere">Théo Lassaunière</a> <br/>as part of the <b>Software security</b> course.</span>
</p> 

SecTrans is a program intended for secure file transfer.  
The program has been built around the imposed `libserver.so` and `libclient.so` libraries.  
[Pre-coding](docs/pre-coding.md) and [post-coding](docs/post-coding.md) reports are available in the `docs` folder.

## Building

```shell
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build --config Release --target all
```

## Usage

### Server

You can launch it with `./build/sectrans_server` and `Ctrl+C` to gracefully stop it.

### Client

You can find the binary at `./build/sectrans_client` and you'll need to start it with one of those arguments.

|      Argument      |             Description              |
|:------------------:|:------------------------------------:|
|  `-up <filepath>`  |     Uploads a file to the server     |
| `-down <filepath>` |   Downloads a file from the server   |
|      `-list`       | Lists the files stored on the server |
