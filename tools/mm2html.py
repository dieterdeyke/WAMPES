#! /usr/bin/env python3

# Author: Dieter Deyke <dieter.deyke@gmail.com>
# Time-stamp: <2015-11-22 10:16:29 deyke>

import os
import re
import sys

HEADERLEVELS = 6
LISTLEVELS = 6

class Header:
    def __init__(self, level, label, text, in_toc, linenum):
        self.level = level
        self.label = label
        self.text = text
        self.in_toc = in_toc
        self.linenum = linenum

class globals: pass
g = globals()

g.headercnt = HEADERLEVELS * [0]
g.headers = []
g.listtype = LISTLEVELS * [0]
g.manual = ""
g.picturecnt = 0
g.title = ""

def error(msg):
    sys.stderr.write("*** ERROR in line %d: %s\n" % (g.linenum, msg))

def find_header_text(text):
    best_header = None
    for header in g.headers:
        if (text == header.text) and (not best_header or abs(best_header.linenum - g.linenum) > abs(header.linenum - g.linenum)):
            best_header = header
    return best_header

def escape_special_characters(s):
    fontswitch = 0
    result = ""
    while s:
        if s.startswith('"'):
            result += "&quot;"
            s = s[1:]
        elif s.startswith("&"):
            result += "&amp;"
            s = s[1:]
        elif s.startswith("<"):
            result += "&lt;"
            s = s[1:]
        elif s.startswith(">"):
            result += "&gt;"
            s = s[1:]
        elif len(s) >= 2 and s.startswith("\\"):
            if s[1] == "e":
                result += "\\"
                s = s[2:]
            elif s[1] == "f":
                if s[2] == "B" or s[2] == "I":
                    if fontswitch:
                        error("font nesting error")
                    resultsave = result
                    result = ""
                    fontswitch = s[2]
                    s = s[3:]
                elif s[2] == "P":
                    if not fontswitch:
                        error("font nesting error")
                    else:
                        text = result
                        result = resultsave
                    s = s[3:]
                    header = find_header_text(text)
                    if fontswitch == "B" and header:
                        result += '<A HREF="#' + header.label + '"><B>' + text + "</B></A>"
                    else:
                        result += "<" + fontswitch + ">" + text + "</" + fontswitch + ">"
                    fontswitch = 0
                else:
                    error("unknown \\ sequence")
                    s = s[1:]
            else:
                error("unknown \\ sequence")
                s = s[1:]
        else:
            result += s[0]
            s = s[1:]
    if fontswitch:
        error("font nesting error")
    return result

def print_contents():
    if g.headers:
        lastlevel = 0
        print("<HR><H2>Table of Contents</H2>")
        for header in g.headers:
            if header.in_toc:
                while lastlevel < header.level:
                    print("<OL>")
                    lastlevel += 1
                while lastlevel > header.level:
                    print("</OL>")
                    lastlevel -= 1
                print("<LI><A HREF=\"#%s\">%s</A>" % (header.label, header.text, ))
        while lastlevel > 0:
            print("</OL>")
            lastlevel -= 1

def start_list(type):
    if g.listlevel < LISTLEVELS:
        print("<%cL>" % (type, ))
        g.listtype[g.listlevel] = type
        g.listlevel += 1
    else:
        error("list nesting error")

def dot_AL(argv):
    start_list("O")

def dot_BL(argv):
    start_list("U")

def dot_LI(argv):
    if g.listlevel <= 0:
        error("list nesting error")
        return
    if g.listtype[g.listlevel - 1] == "D":
        if len(argv) != 2:
            error("incorrect argument count")
            return
        print("<DT>%s\n<DD>" % (argv[1], ), end="")
    elif g.listtype[g.listlevel - 1] == "O":
        print("<LI>", end="")
        if len(argv) != 1:
            error("incorrect argument count")
    elif g.listtype[g.listlevel - 1] == "U":
        print("<LI>", end="")
        if len(argv) != 1:
            error("incorrect argument count")
    else:
        error("unknown list type")

def dot_LE(argv):
    if g.listlevel <= 0:
        error("list nesting error")
        return
    g.listlevel -= 1
    print("</%cL>" % (g.listtype[g.listlevel], ))

def dot_DS(argv):
    print("<PRE>")

def dot_DE(argv):
    print("</PRE>")

def dot_H(argv):
    if len(argv) < 3 or len(argv) > 4:
        error("incorrect argument count")
        return
    level = int(argv[1])
    if level < 1 or level > HEADERLEVELS:
        error("incorrect header level")
        return
    if level > g.headerlevel + 1:
        error("incorrect header level")
    while g.headerlevel < level:
        g.headercnt[g.headerlevel] = 0
        g.headerlevel += 1
    g.headerlevel = level
    g.headercnt[level - 1] += 1
    label = "H"
    for i in range(level):
        label += str(g.headercnt[i])
        if level == 1 or i < level - 1:
            label += "."
    if g.passnumber == 1:
        g.headers.append(Header(level, label, argv[2], level <= g.numbers["Cl"], g.linenum))
    else:
        if not g.afterheader:
            print_contents()
            g.afterheader = 1
        if level == 1:
            print("<HR>", end="")
        print("<H%d><A NAME=\"%s\">%s %s" % (level + 1, label, label[1:], argv[2]), end="")
        if len(argv) > 3:
            print(argv[3], end="")
        print("</A></H%d>" % (level + 1, ))

def dot_P(argv):
    print("<P>")

def dot_PS(argv):
    g.picturecnt += 1
    filename = "%s.%d.gif" % (g.manual, g.picturecnt)
    print('<IMG SRC="%s" ALT="[picture]">' % (os.path.basename(filename),))
    cmd = ("groff -p | " +
           "gs -sDEVICE=pbmraw -r125 -q -dNOPAUSE -sOutputFile=- - | " +
           "pnmcrop -quiet | " +
           "ppmtogif -quiet -transparent rgb:ffff/ffff/ffff > " + filename)
    p = os.popen(cmd, "w")
    p.write(".PS\n")
    while True:
        if not g.lines:
            error("picture nesting error")
            p.write(".PE\n")
            break
        line = g.lines.pop(0)
        g.linenum += 1
        p.write(line)
        if line.startswith(".PE"):
            break
    p.close()

def dot_SP(argv):
    print("<P>")

def dot_TS(argv):

    class Col:
        def __init__(self):
            self.exists = 0
            self.adjust = 0
            self.font = 0

    MAXROW = 16
    MAXCOL = 64

    borderflg = 0
    tab = "\t"

    columns = {}
    for row in range(MAXROW):
        for col in range(MAXCOL):
            columns[row,col] = Col()

    row = 0
    while g.lines:
        line = g.lines.pop(0)
        g.linenum += 1
        if row >= MAXROW:
            error("too many table format rows")
            row = MAXROW - 1
        if ";\n" in line:
            if "box" in line:
                borderflg = 1
            i = line.find("tab(")
            if i > -1:
                tab = line[i + 4]
            continue
        column = 0
        for token in filter(None, re.split("[" + re.escape(" \t.|\n") + "]", line)):
            if column >= MAXCOL:
                error("too many table columns")
                column = MAXCOL - 1
            columns[row,column].exists = 1
            if "l" in token or "L" in token:
                columns[row,column].adjust = "l"
            if "r" in token or "R" in token:
                columns[row,column].adjust = "r"
            if "c" in token or "C" in token:
                columns[row,column].adjust = "c"
            if "n" in token or "N" in token:
                columns[row,column].adjust = "r"
            if "a" in token or "A" in token:
                columns[row,column].adjust = "l"
            if "b" in token or "B" in token:
                columns[row,column].font = "b"
            if "i" in token or "I" in token:
                columns[row,column].font = "i"
            column += 1
        if ".\n" in line:
            break
        row += 1

    if borderflg:
        print("<TABLE BORDER>")
    else:
        print("<TABLE>")

    row = 0
    while g.lines:
        line = g.lines.pop(0)[:-1]
        g.linenum += 1
        if line.startswith(".TE"):
            break
        if line == "_" or line == "=":
            continue
        print("<TR>", end="")
        fields = line.split(tab) + [""] * MAXCOL
        column = 0
        while columns[row,column].exists:
            print("<TD", end="")
            if columns[row,column].adjust == "l":
                print(' ALIGN="left"', end="")
            if columns[row,column].adjust == "c":
                print(' ALIGN="center"', end="")
            if columns[row,column].adjust == "r":
                print(' ALIGN="right"', end="")
            print(">", end="")
            if columns[row,column].font == "b":
                print("<B>", end="")
            if columns[row,column].font == "i":
                print("<I>", end="")
            fields[column] = fields[column].strip()
            if fields[column]:
                print(escape_special_characters(fields[column]), end="")
            else:
                print("&nbsp;", end="")
            if columns[row,column].font == "b":
                print("</B>", end="")
            if columns[row,column].font == "i":
                print("</I>", end="")
            column += 1
        print()
        if row + 1 < MAXROW and columns[row + 1,0].exists:
            row += 1
    print("</TABLE>")

def dot_VL(argv):
    start_list("D")

def dot_ce(argv):
    if len(argv) != 1:
        error("incorrect argument count")
        return
    g.centered = 1

def dot_nr(argv):
    if len(argv) != 3:
        error("incorrect argument count")
        return
    if len(argv[1]) != 2:
        error("illegal register name")
        return
    g.numbers[argv[1]] = int(argv[2])

def do_ignore(argv):
    pass

def not_implemented(argv):
    error("not implemented")

pass1_commandtable = {

    ".H" : dot_H,
    ".nr" : dot_nr,

    "" : do_ignore,

}

pass2_commandtable = {

    ".AL" : dot_AL,
    ".BL" : dot_BL,
    ".DE" : dot_DE,
    ".DS" : dot_DS,
    ".H" : dot_H,
    ".LE" : dot_LE,
    ".LI" : dot_LI,
    ".P" : dot_P,
    ".PF" : do_ignore,
    ".PH" : do_ignore,
    ".PS" : dot_PS,
    ".S" : do_ignore,
    ".SP" : dot_SP,
    ".TC" : do_ignore,
  # ".TE" : do_ignore,
    ".TS" : dot_TS,
    ".VL" : dot_VL,
    '.\\"' : do_ignore,
    ".ce" : dot_ce,
    ".ds" : do_ignore,
    ".ft" : do_ignore,
    ".nr" : dot_nr,

    "" : not_implemented,
}

def reset_state():
    g.afterheader = 0
    g.aftertitle = 0
    g.centered = 0
    g.headerlevel = 0
    g.linenum = 0
    g.listlevel = 0
    g.numbers = {}
    g.numbers["Cl"] = 2

def parse_command_line(line, commandtable):
    argv = []
    while True:
        line = line.lstrip()
        if not line: break
        tmp = ""
        if line[0] == '"':
            line = line[1:]
            while line and line[0] != '"':
                tmp += line[0]
                line = line[1:]
            if line:
                line = line[1:]
        else:
            while line and not line[0].isspace():
                if line[0] == "\\" and line[1] == " ":
                    tmp += " "
                    line = line[2:]
                else:
                    tmp += line[0]
                    line = line[1:]
        if tmp.startswith('\\"'):
            break
        if len(argv):
            argv.append(escape_special_characters(tmp))
        else:
            argv.append(tmp)
    if len(argv):
        commandtable.get(argv[0], commandtable[""])(argv)

def main():

    if len(sys.argv) < 2:
        sys.stderr.write("Usage: " + sys.argv[0] + " file\n")
        return
    filename = sys.argv[1]
    g.manual = sys.argv[1]
    if g.manual.endswith(".mm"):
        g.manual = g.manual[:-3]

    reset_state()
    g.passnumber = 1
    g.lines = open(filename).readlines()
    while g.lines:
        line = g.lines.pop(0)[:-1]
        g.linenum += 1
        if line.startswith("."):
            parse_command_line(line, pass1_commandtable)
        elif not g.title:
            g.title = escape_special_characters(line)
            while True:
                i = g.title.find("<")
                if i == -1: break
                j = g.title.find(">", i)
                g.title = g.title[:i] + g.title[j+1:]

    reset_state()
    g.passnumber = 2
    print("<HTML>")
    if g.title:
        print("<HEAD>")
        print("<TITLE>" + g.title + "</TITLE>")
        print("</HEAD>")
    print("<BODY>")
    g.lines = open(filename).readlines()
    while g.lines:
        line = g.lines.pop(0)[:-1]
        g.linenum += 1
        if line.startswith("."):
            parse_command_line(line, pass2_commandtable)
        else:
            if not g.aftertitle:
                print("<H1>" + g.title + "</H1>")
                g.aftertitle = 1
                g.centered = 0
            else:
                print(escape_special_characters(line))
                if g.centered:
                    print("<BR>")
                    g.centered = 0
    print("</BODY>")
    print("</HTML>")

    if g.listlevel:
        error("list nesting error")

if __name__ == "__main__": main()
