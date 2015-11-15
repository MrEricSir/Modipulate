modipulate-gml
==============

Wrapper layer for Modipulate that is compatible with Game Maker Language (GML). Once compiled with Modipulate, it can be turned into an extension in Game Maker: Studio.

## Why?

[Game Maker][game maker] is a common tool for making games, aimed at entry-level creators. It features a drag 'n' drop GUI as well as its own scripting language, Game Maker Language (**GML**). It also supports C-compatible software libraries when wrapped in _Extensions_, which can be set up and even made cross-platform with a little elbow grease.

Due to the limitations of GML, the only types which may be passed to or returned from functions are `string` and `number`, which correspond to C's `char*` and `double`. modipulate-gml creates an interface layer to Modipulate that Game Maker can work with.

## Usage

1. [Build the Modipulate project][build].
    - Be aware what architecture you build. If you compile Modipulate for 64-bit, you may need to use GM's "YYC" compiler to make your game 64-bit since (as of the writing of this document) the normal game build method will only create 32-bit executables, making it incompatible with 64-bit libraries.
2. Create the extension in your GM project. You have two options:
    1. Manually create the extension in the left panel of your project using [the guide in their documentation][extension guide]. This will require adding all the functions with correct signature (argument and return types).
    2. If you are familiar with how GM stores extensions inside your project folder, you can copy the skeleton `Modipulate.extension.gmx` file from this source to your project tree. The functions have been defined for you, but may need to edit the file as necessary. You will also also have to copy over the built library file. Your project tree should look similar to this:
    
    ```
    mygame.gmx/
        ...
        mygame.project.gmx
        extensions/
            Modipulate.extension.gmx
            Modipulate/
                modipulategml.dll
    ```

## Caveats

How behavior of this layer differs from base Modipulate:

- The only data types used are `double` and `char*`.
- A negative return value indicates an error.
    - This is to avoid confusion when calling functions which should return a positive number (e.g., `*_set_volume()` and `*_load_song()`).
    - The only exception is `modipulategml_global_init()`, which returns `-1000` upon success. Since GML initializes variables to `0`, failure to load the library would be hard to catch with a standard success value of `0`. Assert that this function returns `-1000` after calling.
- Error strings are obtained with `modipulategml_error_to_string(errno)`.
- Songs are referenced with _ID numbers_ instead of pointers.
- Songs IDs are automatically assigned upon loading a song, and freed upon unloading.
- Functions which take an on/off flag have been split up into separate functions: `*_song_play()` and `*_song_stop()`, `*_enable()` and `*_disable()`, etc.

## About Modipulate

Modipulate is a library for playing and dynamically manipulating MOD-style music inside of games. See the [home page][home] for details.

## License

Copyright 2015 Eric Gregory and Stevie Hryciw.

Modipulate is released under a BSD license. See the [LICENSE file][license] for more information.

[game maker]: http://www.yoyogames.com/studio
[build]: https://github.com/MrEricSir/Modipulate/wiki/Building-Modipulate
[home]: https://github.com/MrEricSir/Modipulate/
[license]: https://github.com/MrEricSir/Modipulate/blob/master/LICENSE
[extension guide]: http://docs.yoyogames.com/source/dadiospice/001_advanced%20use/extensions/creating%20extensions.html
