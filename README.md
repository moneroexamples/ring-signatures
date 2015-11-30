# Ring signatures

 [Monero](https://getmonero.org/)

In this example, this problem is addressed. Specifically, a C++ program called
`showmixins` is developed. The program prints out all mixins used in a given transaction. In addition,
if monero address and privet view key are provided,
the true mixins will be identified.

## Pre-requisites

Everything here was done and tested
on Ubuntu 14.04 x86_64 and Ubuntu 15.10 x86_64.

Instruction for Monero compilation:
 - [Ubuntu 14.04 x86_64](http://moneroexamples.github.io/compile-monero-ubuntu/)
 - [Ubuntu 15.10 x86_64](http://moneroexamples.github.io/compile-monero-ubuntu-1510/)

Monero source code compilation and setup are same as
[here](http://moneroexamples.github.io/access-blockchain-in-cpp/).



## C++ code
The main part of the example is main.cpp.

```c++


## Program options

```
./rings -h
```

## Example input and output


## Compile this example
The dependencies are same as those for Monero, so I assume Monero compiles
correctly. If so then to download and compile this example, the following
steps can be executed:

```bash
# download the source code
git clone https://github.com/moneroexamples/ring-signatures.git

# enter the downloaded sourced code folder
cd ring-signatures

# create the makefile
cmake .

# compile
make
```

After this, `rings` executable file should be present in access-blockchain-in-cpp
folder. How to use it, can be seen in the above example outputs.


## How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.

Some Monero are also welcome:
```
48daf1rG3hE1Txapcsxh6WXNe9MLNKtu7W7tKTivtSoVLHErYzvdcpea2nSTgGkz66RFP4GKVAsTV14v6G3oddBTHfxP6tU
```
