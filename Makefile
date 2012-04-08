# TODO: try to find ttf2eot and ttfautohint globally installed first

PROJECT     := $(notdir ${PWD})
FONT_NAME   := entypo


################################################################################
## ! DO NOT EDIT BELOW THIS LINE, UNLESS YOU REALLY KNOW WHAT ARE YOU DOING ! ##
################################################################################


TMP_PATH    := /tmp/${PROJECT}-$(shell date +%s)
REMOTE_NAME ?= origin
REMOTE_REPO ?= $(shell git config --get remote.${REMOTE_NAME}.url)


# Add local versions of ttf2eot nd ttfautohint to the PATH
PATH := $(PATH):./support/font-builder/support/ttf2eot
PATH := $(PATH):./support/font-builder/support/ttfautohint/frontend
PATH := $(PATH):./support/font-builder/bin


dist: font html


font:
	@if test ! -d support/font-builder/bin ; then \
		echo "font-builder binaries not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
	@if test ! `which ttf2eot` ; then \
		echo "ttf2eot not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
	@if test ! `which ttfautohint` ; then \
		echo "ttfautohint not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
	fontbuild.py -c ./config.yml -t ./src/font_template.sfd -i ./src/svg -o ./font/$(FONT_NAME).ttf
	ttfautohint --latin-fallback --hinting-limit=200 --hinting-range-max=50 --symbol ./font/$(FONT_NAME).ttf ./font/$(FONT_NAME)-hinted.ttf
	mv ./font/$(FONT_NAME)-hinted.ttf ./font/$(FONT_NAME).ttf
	fontconvert.py -i ./font/$(FONT_NAME).ttf -o ./font
	ttf2eot < ./font/$(FONT_NAME).ttf >./font/$(FONT_NAME).eot


support:
	git submodule init support/font-builder
	git submodule update support/font-builder
	which ttf2eot ttfautohint > /dev/null || (cd support/font-builder && $(MAKE))


html:
	parse_template.py -c ./config.yml ./src/css.mustache ./font/entypo.css
	parse_template.py -c ./config.yml ./src/demo.mustache ./font/demo.html


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
