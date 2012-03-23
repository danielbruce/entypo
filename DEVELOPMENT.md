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

1. Place images into `/src/svg` folder.
2. Add image info to `config.yml` (see comments in it)
3. Edit css/html templates, if needed.
4. Run `make`

Generated data will be placed in `./font`
