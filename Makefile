# TODO: try to find ttf2eot and ttfautohint globally installed first

PROJECT     :=  $(notdir ${PWD})
TMP_PATH    := /tmp/${PROJECT}-$(shell date +%s)

REMOTE_NAME ?= origin
REMOTE_REPO ?= $(shell git config --get remote.${REMOTE_NAME}.url)


TTF2EOT_BIN     = ./support/ttf2eot/ttf2eot
TTFAUTOHINT_BIN = ./support/ttfautohint/frontend/ttfautohint


dist: font html


font:
	@if test ! -f $(TTF2EOT_BIN) ; then \
		echo "ttf2eot not found. run:" >&2 ; \
		echo "  make support" >&2 ; \
		exit 128 ; \
		fi
#	@if test ! -f $(TTFAUTOHINT_BIN) ; then \
#		echo "ttfautohint not found. run:" >&2 ; \
#		echo "  make support" >&2 ; \
#		exit 128 ; \
#		fi
	./bin/fontbuild.py -c ./config.yml -t ./src/font_template.sfd -i ./src/svg -o ./font/entypo.ttf
	$(TTFAUTOHINT_BIN) --latin-fallback --hinting-limit=200 --hinting-range-max=50 --symbol ./font/entypo.ttf ./font/entypo-hinted.ttf \
		&& mv ./font/entypo-hinted.ttf ./font/entypo.ttf
	./bin/fontconvert.py -i ./font/entypo.ttf -o ./font
	$(TTF2EOT_BIN) < ./font/entypo.ttf >./font/entypo.eot


support: $(TTF2EOT_BIN) $(TTFAUTOHINT_BIN)


$(TTF2EOT_BIN):
	cd ./support/ttf2eot \
		&& $(MAKE) ttf2eot


$(TTFAUTOHINT_BIN):
	cd ./support/ttfautohint \
		&& ./configure --without-qt \
		&& make
	git clean -f -d ./support/ttfautohint


html:
	./bin/parse_template.py -c ./config.yml ./src/css.mustache ./font/entypo.css
	./bin/parse_template.py -c ./config.yml ./src/demo.mustache ./font/demo.html


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


dev-deps:
	@if test 0 -ne `id -u` ; then \
		echo "root priveledges are required" >&2 ; \
		exit 128 ; \
		fi
	apt-get -qq install \
		fontforge python python-fontforge libfreetype6-dev \
		python-yaml python-pip \
		build-essential \
		autoconf automake libtool
	pip -q install pystache argparse


clean:
	git clean -f -x


.SILENT: dev-deps
.PHONY: font support
