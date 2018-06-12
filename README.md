# NetworkBot
A bot for the Sc2LadderServer system that connects over a network.

# !Currently not working!
Note that the bot does not work at this time because I had something hard-coded in my test environment that allowed the bot to work - I intend to resolve this issue.

# Running the bot
The bot accepts parameters via the following commandline arguments:

| Command | Short Command | Description |
|---|---|---|        
| `--LadderServer`  |`-l`|	Ladder server address   |
| `--GamePort`      |`-g`|	Port to connect to      |
| `--StartPort`     |`-i`|	Starting server port    |

Example:
```
NetworkBot.exe --LadderServer 192.168.1.138 --GamePort 5677 --StartPort 5690
```

# Developer Install / Compile Instructions
## Requirements
* [CMake](https://cmake.org/download/)
* Starcraft 2 ([Windows](https://starcraft2.com/en-us/)) ([Linux](https://github.com/Blizzard/s2client-proto#linux-packages)) 
* [Starcraft 2 Map Packs](https://github.com/Blizzard/s2client-proto#map-packs) 

## Windows

* Download and install [Visual Studio 2017](https://www.visualstudio.com/downloads/) if you need it.

```bat
:: Clone the project
$ git clone --recursive https://github.com/lladdy/NetworkBot
$ cd NetworkBot

:: Create build directory.
$ mkdir build
$ cd build

:: Generate VS solution.
$ cmake ../ -G "Visual Studio 15 2017 Win64"

:: Build the project using Visual Studio.
$ start NetworkBot.sln
```

 ### Linux and OS X
 
 ```bash
 # Clone the project.
 $ git clone --recursive https://github.com/lladdy/NetworkBot
 $ cd NetworkBot
 
 # Create build directory.
 $ mkdir build
 $ cd build
 
 # Generate a Makefile.
 $ cmake ../
 
 # Build.
 $ make
 ```

### Submodules
If you don't initially do a `--recursive` clone (submodule folders will be left empty), you can download any submodules later like so:
```
git submodule update --init --recursive
```
Alternatively, you could opt to symlink the folder of the submodule in question to an existing copy already on your computer. However, note that you will very likely be using a different version of the submodule to that which would otherwise be downloaded in this repository, which could cause issues (but it's probably not too likely). 
