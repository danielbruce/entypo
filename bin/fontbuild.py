#!/usr/bin/env python

import argparse
import yaml
import fontforge

KERNING = 15

parser = argparse.ArgumentParser(description='Font builder tool')
parser.add_argument('-c', '--config', type=str, help='Config example: ../config.yml', required=True)
parser.add_argument('-t', '--sfd_template', type=str, help='SFD template file', required=True)
parser.add_argument('-i', '--svg_dir', type=str, help='Input svg file', required=True)
parser.add_argument('-o', '--ttf_file', type=str, help='Output ttf file', required=True)

args = parser.parse_args()

config = yaml.load(open(args.config, 'r'))

#font = fontforge.open(args.sfd_template)
font = fontforge.font()

# set font params
font.version = config['font']['version']
font.fontname = config['font']['fontname']
font.fullname = config['font']['fullname']
font.familyname = config['font']['familyname']
font.copyright = config['font']['copyright']
font.ascent = config['font']['ascent']
font.descent = config['font']['descent']
font.weight = config['font']['weight']

# process glyphs
for item in config['glyphs']:
    name = item.keys()[0]
    glyph = item[name]

    c = font.createChar(int(glyph['code']))

    c.importOutlines(args.svg_dir + '/' + name + '.svg')
    c.left_side_bearing = KERNING
    c.right_side_bearing = KERNING

    # small optimization, should not affect quality
    c.simplify()
    c.round()
#    c.addExtrema()

font.generate(args.ttf_file)
