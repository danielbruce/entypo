# Copyright 2011 Nicolai Stange
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_DEFUN([LT_LTLIZE_LANG],
[AC_REQUIRE([LT_OUTPUT])]
[AC_LANG_DEFINE([LTLIZED $1], 
[lt_[]_AC_LANG_DISPATCH([_AC_LANG_ABBREV], [$1])], 
[LT_[]_AC_LANG_DISPATCH([_AC_LANG_PREFIX], [$1])],
[_AC_LANG_DISPATCH([_AC_CC], [$1])], [$1],
[_AC_LANG_DISPATCH([AC_LANG], [$1])])]
[m4_append([AC_LANG(LTLIZED $1)],
[ac_link="$ac_compile; ./libtool --mode=link `echo $ac_link | sed 's/\$ac_ext/\$ac_objext/'`"])]
[m4_ifdef([AC_LANG_COMPILER($1)],
[m4_copy([AC_LANG_COMPILER($1)], [AC_LANG_COMPILER(LTLIZED $1)])])]
)dnl
