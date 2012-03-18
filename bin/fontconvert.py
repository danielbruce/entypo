#!/usr/bin/env python

import argparse
import fontforge
import os

parser = argparse.ArgumentParser(description='Font convertor tool')
parser.add_argument('-i', '--src_font', type=str, help='Input font file', required=True)
parser.add_argument('-o', '--fonts_dir', type=str, help='Output fonts folder', required=True)

args = parser.parse_args()

font_name = os.path.basename(args.src_font)[:-4]
font_name_template = args.fonts_dir + '/' + font_name

font = fontforge.open(args.src_font)

font.generate(font_name_template + '.otf')
font.generate(font_name_template + '.woff')
font.generate(font_name_template + '.svg')

