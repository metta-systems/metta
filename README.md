![Metta OS](https://raw.github.com/berkus/metta/master/docs/metta.png)

Metta is a multimedia, mobile, social OS
========================================

My goal is to make Metta the platform for social, efficient and fun life on the internet. I call such internet egocentric, because it revolves around your needs and desires. See a more detailed description at http://atta-metta.net.

Be free!

मेता

-----------------------------------------

To build Metta
==============

 * Check out the sources:

```
 $ mkdir Metta; cd Metta
 $ git clone https://github.com/berkus/metta.git develop
 $ cd develop; git checkout develop
```

Sources will be checked out into branch "develop" under "Metta". This extra umbrella directory is needed because toolchain builder will create Metta/toolchain for the local toolchain it builds.

 * Install dependencies

  * yasm assembler, `brew install yasm` for example.
  * boost, `brew install boost`
  * OSSP uuid implementation, `brew install ossp-uuid`
  * up-to-date openssl, `brew install openssl`
  * cdrtools (for mkisofs), `brew install cdrtools`
  * cmake, `brew install cmake`
  * ninja, `brew install ninja`
  * bochs emulator, `brew install bochs`

All dependencies in one command:
```
 $ brew install yasm boost ossp-uuid openssl cdrtools cmake ninja bochs
```

 * Generate a toolchain.

```
 $ cd Metta
 $ sh develop/build_toolchain.sh
```

This is going to take a while.

If you're unable to build toolchain locally and are on a (post-) Lion Mac, download prebuilt one and unpack it.

```
 $ cd Metta
 $ wget https://r.atta-metta.net/downloads/toolchain-x86_64-darwin.tar.bz2
 $ tar xf toolchain-x86_64-darwin.tar.bz2
```

 * Build Metta

```
 $ cd Metta/develop/src
 $ sh buildit.sh
```

 * After successful build run emulator software to try out Metta.

src directory is preconfigured for using Bochs, so you can simply type:

```
 $ bochs -q
```

-----------------------------------------

You are free to contribute and remember: if you do not, somebody else will!
Just send a pull request on github.

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/berkus/metta/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

[Build & test status dashboard](https://github.com/berkus/metta/wiki/Dashboard)
