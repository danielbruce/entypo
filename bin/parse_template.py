#!/usr/bin/env python

import argparse
import yaml
import pystache

parser = argparse.ArgumentParser(description='Generate file by mustache template')
parser.add_argument('-c', '--config', type=str, help='Config example: ../config.yml', required=True)
parser.add_argument('template', type=str, help='Mustache template')
parser.add_argument('output', type=str, help='Output file')

args = parser.parse_args()

config = yaml.load(open(args.config, 'r'))

glyphs = []
for name, glyph_info in config['glyphs'].iteritems():
    # use giph name if css field is absent
    if (not 'css' in glyph_info):
        glyph_info['css'] = name

    # code to sting in hex format
    glyph_info['code'] = hex(glyph_info['code'])

    glyph_info['name'] = name

    glyphs.append(glyph_info)

template = open(args.template, 'r').read()

output = pystache.render(template, {'glyphs': glyphs})

f = open(args.output, 'w')
f.write(output)
