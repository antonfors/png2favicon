# png2favicon
Combines one or more PNG files into a ```favicon.ico``` file. Useful in order to utilize optimized PNG files to reduce the size of the ```favicon.ico``` file on websites, to reduce overhead on the network and increase loading speed for end users. The ```favicon.ico``` file is almost always requested by web browsers, yet often overlooked in optimization efforts.

## Compilation
```
cc -Os png2favicon.c -o png2favicon
```

## Usage
Have your icon ready saved in the PNG format in the sizes you want, the most common combination for the web is 16×16, 32×32 and 48×48 pixels. Then use a PNG optimizer such as ```ZopfliPNG``` or ```OptiPNG``` to reduce the size of the PNG files. Then just invoke ```png2favicon``` like this:

```
png2favicon 16x16.png 32x32.png 48x48.png
```

Now you should have a well optimized ```favicon.ico``` file in your working directory, ready for the world wide web!
