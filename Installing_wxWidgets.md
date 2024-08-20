# About

This file is created to display the installation procedure for wxWidgets followed before using wx-flags. A nonmonolithic, static, debug build is created. 

# On MSYS2: 
```sh
pacman -Syu
mkdir -p ~/wxWidgets-build
cd ~/wxWidgets-build
curl -L -o wxWidgets-3.2.5.zip https://github.com/wxWidgets/wxWidgets/archive/refs/tags/v3.2.5.zip
unzip wxWidgets-3.2.5.zip
cd wxWidgets-3.2.5
mkdir build-debug
cd build-debug
curl -L -o catch2.zip https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.10.zip
mkdir -p ../3rdparty/catch/include
mv Catch2-2.13.10/single_include/catch2/catch.hpp ../3rdparty/catch/include/
curl -L -o nanosvg.zip https://github.com/memononen/nanosvg/archive/refs/heads/master.zip
unzip nanosvg.zip
mkdir -p ../3rdparty/nanosvg
mv nanosvg-master/* ../3rdparty/nanosvg/
curl -L -o pcre-8.44.tar.gz https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz/download
tar -xzf pcre-8.44.tar.gz
mv pcre-8.44 ../3rdparty/pcre/
../configure --disable-shared --enable-debug --disable-monolithic
```


## Expected Result: 
The `../configure` command should end with: 

<pre>
 Configured wxWidgets 3.2.5 for `x86_64-w64-mingw32'
 
  Which GUI toolkit should wxWidgets use?                 msw
  Should wxWidgets be compiled into single library?       no
  Should wxWidgets be linked as a shared library?         no
  Should wxWidgets support Unicode?                       yes (using wchar_t)
  What level of wxWidgets compatibility should be enabled?
                                       wxWidgets 2.8      no
                                       wxWidgets 3.0      yes
  Which libraries should wxWidgets use?
                                       STL                no
                                       jpeg               sys
                                       png                sys
                                       regex              sys
                                       tiff               sys
                                       lzma               no
                                       zlib               sys
                                       expat              sys
                                       libmspack          no
                                       sdl                no
</pre>

## Removal of downloaded 3rd party archives: 
```sh
rm pcre-8.44.tar.gz
rm catch2.zip
rm -rf Catch2-2.13.10/
rm nanosvg.zip
rm -rf nanosvg-master/
```

## Final Step
Run `mingw32-make`. 
