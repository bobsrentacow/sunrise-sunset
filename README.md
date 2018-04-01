# sunrise-sunset
linux shared library to fetch sunrise &amp; sunset data from api.sunrise-sunset.org

Install
-------

1) Install git

    $ sudo apt-get install git

2) Clone and build jsmn

    $ git clone https://github.com/zserge/jsmn
  
    $ cd jsmn
  
    $ make
    
    $ cd ..
  
    This should create jsmn.o

3) clone sunrise-sunset repository

    $ git clone https://github.com/bobsrentacow/sunrise-sunset
    
4) copy jsmn header and library

    $ cp jsmn/jsmn.o sunrise-sunset/
    
    $ cp jsmn/jsmn.h sunrise-sunset/
    
5) build sunrise-sunset

    $ cd sunrise-sunset
    
    $ make

6) install the shared library into /usr/lib and update the library cache

    $ sudo make install

Test
-------

After completing the Install steps, execute the test executable under sunrise-sunset.  
