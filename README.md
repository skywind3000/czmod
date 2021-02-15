# Preface
 
[z.lua](https://github.com/skywind3000/z.lua) is fast enough for most cases, the path tracking action will be triggered each time when you change your current directory.

So I still recommend the pure lua script for portability and flexibility, but for someone who really cares about very high performance, this module can be helpful.


## Features

- Speeds up `z.lua` for history tracking and matching.
- Easy to install.

## Install

### Install musl-gcc

```bash
sudo apt-get install musl-tools
```

### Build the binary

```Bash
git clone https://github.com/skywind3000/czmod.git ~/github/czmod
cd ~/github/czmod
sh build.sh
```

### Enable Czmod

`czmod` must be initialized after `z.lua`:

bash:

```bash
eval "$(lua ~/github/z.lua/z.lua --init bash enhanced once echo)"
source ~/github/czmod/czmod.bash
```

zsh:

```bash
eval "$(lua ~/github/z.lua/z.lua --init zsh enhanced once echo)"
source ~/github/czmod/czmod.zsh
```

## Benchmark

Average performance:

| Name | czmod |  z.lua |
|-|-|-|
| **Update Time** | 1.6ms | 13.2ms |
| **Query Time** | 1.5ms | 9.8ms |

## Credit

TODO
