Very quick commandline wrapper around OpenTypeUtilities.cpp from Chromium, used
to make EOT (Embeddable Open Type) files from TTF (TrueType/OpenType Font)
files. This is the format TTLoadEmbeddedFont() accepts, which is what Internet
Explorer uses for css @font-face declarations.

I've only tested this on Linux.

EOT was documented by Microsoft here:
    <http://www.w3.org/Submission/2008/SUBM-EOT-20080305/>

TTLoadEmbeddedFont is described here:
    <http://msdn.microsoft.com/en-us/library/dd145155(VS.85).aspx>

Chromium:
    <http://src.chromium.org/viewvc/chrome/trunk/deps/third_party/WebKit/WebCore/platform/graphics/win/OpenTypeUtilities.cpp?view=log&pathrev=7591>

To build:

    $ make

Usage:

    $ ./ttf2eot < input.ttf > output.eot

Author: taviso@sdf.lonestar.org 15-Mar-2009
License: Derived from WebKit, so BSD/LGPL 2/LGPL 2.1.

Keywords for anyone having as much pain as me finding a utility to do this on Linux:

    covert eot to ttf
    eot converter
    wtf is an eot file


TODO: MTX support?
