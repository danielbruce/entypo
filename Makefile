dist:
	./bin/fontbuild.py -c ./config.yml -i ./src/svg -o ./font/entypo.ttf
	#autohint will be here
	./bin/fontconvert.py -i ./font/entypo.ttf -o ./font
	./bin/parse_template.py -c ./config.yml ./src/css.mustache ./font/entypo.css
	./bin/parse_template.py -c ./config.yml ./src/demo.mustache ./font/demo.html

html:
	./bin/parse_template.py -c ./config.yml ./src/css.mustache ./font/entypo.css
	./bin/parse_template.py -c ./config.yml ./src/demo.mustache ./font/demo.html

dev-deps:
	if test 0 -ne `id -u` ; then \
		echo "root priveledges are required" >&2 ; \
		exit 128 ; \
		fi
	apt-get -qq install \
		fontforge python python-fontforge python-argparse python-yaml python-pip
	pip -q install pystache


.SILENT: dev-deps
