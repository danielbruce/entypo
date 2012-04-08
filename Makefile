# TODO: try to find ttf2eot and ttfautohint globally installed first

PROJECT     := $(notdir ${PWD})
FONT_NAME   := entypo


################################################################################
## ! DO NOT EDIT BELOW THIS LINE, UNLESS YOU REALLY KNOW WHAT ARE YOU DOING ! ##
################################################################################


TMP_PATH    := /tmp/${PROJECT}-$(shell date +%s)
REMOTE_NAME ?= origin
REMOTE_REPO ?= $(shell git config --get remote.${REMOTE_NAME}.url)


FONTBUILD_BIN   = ./support/font-builder/bin/fontbuild.py
FONTCONVERT_BIN = ./support/font-builder/bin/fontconvert.py
PARSE_TPL_BIN   = ./support/font-builder/bin/parse_template.py
TTF2EOT_BIN     = ./support/font-builder/support/ttf2eot/ttf2eot
TTFAUTOHINT_BIN = ./support/font-builder/support/ttfautohint/frontend/ttfautohint


dist: font html


font:
	@if test ! -f $(TTF2EOT_BIN) ; then \
		echo "ttf2eot not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
	@if test ! -f $(TTFAUTOHINT_BIN) ; then \
		echo "ttfautohint not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
	$(FONTBUILD_BIN) -c ./config.yml -t ./src/font_template.sfd -i ./src/svg -o ./font/$(FONT_NAME).ttf
	$(TTFAUTOHINT_BIN) --latin-fallback --hinting-limit=200 --hinting-range-max=50 --symbol ./font/$(FONT_NAME).ttf ./font/$(FONT_NAME)-hinted.ttf
	mv ./font/$(FONT_NAME)-hinted.ttf ./font/$(FONT_NAME).ttf
	$(FONTCONVERT_BIN) -i ./font/$(FONT_NAME).ttf -o ./font
	$(TTF2EOT_BIN) < ./font/$(FONT_NAME).ttf >./font/$(FONT_NAME).eot


support:
	git submodule init support/font-builder
	git submodule update support/font-builder
	cd support/font-builder && $(MAKE)


html:
	$(PARSE_TPL_BIN) -c ./config.yml ./src/css.mustache ./font/entypo.css
	$(PARSE_TPL_BIN) -c ./config.yml ./src/demo.mustache ./font/demo.html


gh-pages:
	@if test -z ${REMOTE_REPO} ; then \
		echo 'Remote repo URL not found' >&2 ; \
		exit 128 ; \
		fi
	cp -r ./font ${TMP_PATH} && \
		touch ${TMP_PATH}/.nojekyll
	cd ${TMP_PATH} && \
		git init && \
		git add . && \
		git commit -q -m 'refreshed gh-pages'
	cd ${TMP_PATH} && \
		git remote add remote ${REMOTE_REPO} && \
		git push --force remote +master:gh-pages 
	rm -rf ${TMP_PATH}


.SILENT: dev-deps
.PHONY: font support
