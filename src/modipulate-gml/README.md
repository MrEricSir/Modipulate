modipulate-gml
==============

Wrapper layer for Modipulate that is compatible with Game Maker Language (GML). Once compiled with Modipulate, it can be turned into an extension in Game Maker: Studio.

## Why?

[Game Maker][game maker] is a common tool for making games, aimed at entry-level creators. It features a drag 'n' drop GUI as well as its own scripting language, **GML**.

Due to the type limitations of GML, the only types which may be passed to or returned from functions are `string` and `number`. modipulate-gml creates an interface layer to Modipulate that Game Maker can work with.

## Usage

1. [Build the Modipulate project][build].
    - Be aware what architecture you build. If you compile Modipulate for 64-bit, you may need to use GM's "YYC" compiler to make your game 64-bit since (as of the writing of this document) the normal game build method will only create 32-bit executables, making it incompatible with the library.
2. Add the compiled library (e.g., `libmodipulate.dylib` on OS X) to the "Extensions" tree in your Game Maker project.
3. **(More info coming soon)**

## Caveats

How behavior of this layer differs from base Modipulate:

- The only data types used are `double` and `char*`.
- A negative return value indicates an error.
- Error strings are obtained with `modipulategml_error_to_string(errno)`.
- Songs are referenced with _ID numbers_ instead of pointers.
- Songs IDs are automatically assigned upon loading a song, and freed upon
  unloading.

## About Modipulate

Modipulate is a library for playing and dynamically manipulating MOD-style music inside of games. See the [home page][home] for details.

## License

Copyright 2015 Eric Gregory and Stevie Hryciw.

Modipulate is released under a BSD license. See the [LICENSE file][license] for more information.

[game maker]: http://www.yoyogames.com/studio
[build]: https://github.com/MrEricSir/Modipulate/wiki/Building-Modipulate
[home]: https://github.com/MrEricSir/Modipulate/
[license]: https://github.com/MrEricSir/Modipulate/blob/master/LICENSE
