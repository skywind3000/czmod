# Preface
 
[z.lua](https://github.com/skywind3000/z.lua) is fast enough for most case, the path tracking action will be triggered each time when you change your current directory.

So I still recommend the pure lua script for portability and flexibility, but for someone who really care about `10ms` or `1ms` things, this module can help them to gain the ultimate speed.


## Feature

- Speed up `z.lua` for history tracking and matching.
- **20 times** smaller than `zoxide` in binary size.
- **4-5 times** faster than the rust project `zoxide`.
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

### Enable the czmod

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

| Name | czmod | zoxide | z.lua | autojump |
|-|-|-|-|-|
| **Update Time** | 1.6ms | 7.1ms | 13.2ms | 85.2ms |
| **Query Time** | 1.5ms | 5.8ms | 9.8ms | 79.3ms | 
| **Binary Size** | 102 KB | 2.2 MB | 70 KB | 100 KB |


Benchmark with `hyperfine` for updating:

![](https://skywind3000.github.io/images/p/czmod/i-add.png)

and searching:

![](https://skywind3000.github.io/images/p/czmod/i-query.png)

benchmark of `z.lua` script (update & search):

![](https://skywind3000.github.io/images/p/czmod/zlua.png)

benchmark of `autojump` script (update & search):

![](https://skywind3000.github.io/images/p/czmod/autojump.png)


## Credit

TODO
