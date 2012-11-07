Metta is a multimedia, mobile, social OS
========================================

My goal is to make Metta the platform for social, efficient and fun life on the internet. I call such internet egocentric, because it revolves around your needs and desires. See a more detailed description at http://berkus.github.com.

Be free!

-----------------------------------------

To build Metta
==============

0. Check out the sources:

```
 $ mkdir Metta; cd Metta
 $ git clone https://github.com/berkus/metta.git master
```

Sources will be checked out into branch "master" under "Metta". This extra umbrella directory is needed because toolchain builder will create Metta/toolchain for the local toolchain it builds.

1. Install yasm assembler, 'brew install yasm' for example.

2. Generate a toolchain.

```
 $ cd Metta
 $ sh master/build_toolchain.sh
```

This is going to take a while.

If you're unable to build toolchain locally and are on a (post-) Lion Mac, download prebuilt one and unpack it.

```
 $ cd Metta
 $ wget http://downloads.exquance.com/toolchain-x86_64-darwin.tar.bz2
 $ tar xf toolchain-x86_64-darwin.tar.bz2
```

3. Build Metta

```
 $ cd Metta/master/src
 $ ./waf
```

4. After successful build run emulator software to try out Metta.

src directory is preconfigured for using Bochs, so you can simply type:

```
 $ bochs -q
```

-----------------------------------------

You are free to contribute and remember: if you don't, somebody else will!
Just send a pull request on github.
