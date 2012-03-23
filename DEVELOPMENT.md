Development docs
================


Installation
------------

### Ubuntu

Install dependencies (fontforge & python modules):

    sudo make dev-deps

Build additional software:

    make support


### Mac

TBD. Anyone, please help


### Windows

TBD. Anyone, please help


Making font
-----------

Place images into `/src/svg` folder. Edit css/html templates, if needed. Then run

    make

Generated data will be placed in `./font`
