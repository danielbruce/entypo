#!/usr/bin/env python

import argparse
import yaml
import pystache
import math

parser = argparse.ArgumentParser(description='Generate file by mustache template')
parser.add_argument('-c', '--config', type=str, help='Config example: ../config.yml', required=True)
parser.add_argument('template', type=str, help='Mustache template')
parser.add_argument('output', type=str, help='Output file')

args = parser.parse_args()

data = yaml.load(open(args.config, 'r'))

glyphs = []

for item in data['glyphs']:
    name = item.keys()[0]
    glyph_info = item[name]

    # use giph name if css field is absent
    if (not 'css' in glyph_info):
        glyph_info['css'] = name

    # code to sting in hex format
    glyph_info['code'] = hex(glyph_info['code'])[2:]

    glyphs.append(glyph_info)

data['glyphs'] = glyphs

chunk_size = int(math.ceil(len(glyphs) / float(data['demo-columns'])))
data['columns'] = [{'glyphs': glyphs[i:i + chunk_size]} for i in range(0, len(glyphs), chunk_size)]

template = open(args.template, 'r').read()

output = pystache.render(template, data)

f = open(args.output, 'w')
f.write(output)
