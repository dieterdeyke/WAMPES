# Author: Dieter Deyke <dieter.deyke@gmail.com>
# Time-stamp: <2015-11-30 12:57:25 deyke>

import os
import re
import stat

force_32_bit_build = False
if force_32_bit_build:
    env = Environment(parse_flags = "-Ilib /usr/lib/i386-linux-gnu/libgdbm.so.3 /usr/lib/i386-linux-gnu/libgdbm_compat.so.3 ") # this is a hack !!!!!!!!!!!!!!!!!!!!
    env["CFLAGS"] = "-m32"
    env["LINKFLAGS"] = "-m32"
else:
    env = Environment(parse_flags = "-Ilib -lgdbm -lgdbm_compat") # this is a hack !!!!!!!!!!!!!!!!!!!!

# lib

def findfile(symbol, values):
    for value in values:
        if os.path.exists(value) and not os.path.islink(value):
            conf.Define(symbol, '"' + value +'"')
            return
    conf.Define(symbol, '""')

def findprog(symbol, values):
    for value in values:
        if os.path.exists(value) and (os.stat(value).st_mode & stat.S_IXUSR) and not os.path.islink(value):
            conf.Define(symbol, '"' + value +'"')
            return
        if os.path.exists(value) and (os.stat(value).st_mode & stat.S_IXUSR):
            conf.Define(symbol, '"' + value +'"')
            return
        if os.path.exists(value) and not os.path.islink(value):
            conf.Define(symbol, '"' + value +'"')
            return
        if os.path.exists(value):
            conf.Define(symbol, '"' + value +'"')
            return
    conf.Define(symbol, '""')

def finddir(symbol, values):
    for value in values:
        if os.path.isdir(value) and not os.path.islink(value):
            conf.Define(symbol, '"' + value +'"')
            return
        if os.path.isdir(value):
            conf.Define(symbol, '"' + value +'"')
            return
    conf.Define(symbol, '""')

conf = Configure(env, config_h="lib/configure.h")

finddir("HOME_DIR", ["/home",
                     "/users",
                     "/usr/people",
                     "/u", ])

finddir("NEWS_DIR", ["/usr/local/news/spool",
                     "/var/spool/news",
                     "/usr/spool/news", ])

finddir("UUCP_DIR", ["/var/spool/uucp",
                     "/usr/spool/uucp", ])

findfile("UTMP__FILE", ["/var/run/utmp",
                        "/var/adm/utmp",
                        "/etc/utmp", ])

findfile("WTMP__FILE", ["/var/log/wtmp",
                        "/var/adm/wtmp",
                        "/usr/adm/wtmp",
                        "/etc/wtmp", ])

findprog("CTLINND_PROG", ["/usr/local/news/bin/ctlinnd",
                          "/usr/lib/news/bin/ctlinnd",
                          "/usr/bin/ctlinnd", ])

findprog("LS_PROG", ["/usr/bin/ls",
                     "/bin/ls", ])

findprog("MAIL_PROG", ["/usr/openwin/bin/xview/mail",
                       "/usr/bin/mailx",
                       "/bin/mailx",
                       "/usr/sbin/Mail",
                       "/usr/bin/mail",
                       "/bin/mail", ])

findprog("RNEWS_PROG", ["/usr/local/news/bin/rnews",
                        "/usr/local/bin/rnews",
                        "/usr/bin/rnews",
                        "/usr/lib/news/rnews",
                        "/usr/lib/rnews", ])

findprog("SENDMAIL_PROG", ["/usr/sbin/sendmail",
                           "/usr/lib/sendmail", ])

conf.CheckCHeader("ndbm.h")
conf.CheckCHeader("db1/ndbm.h")
conf.CheckCHeader("gdbm-ndbm.h")

conf.CheckFunc("adjtime")
conf.CheckFunc("strdup")
conf.CheckFunc("strerror")
conf.CheckFunc("strtoul")

if conf.CheckType("fd_set", "#include <sys/types.h>"):
    conf.Define("TYPE_FD_SET", "fd_set")
else:
    conf.Define("TYPE_FD_SET", "struct fd_set")

env = conf.Finish()

env.StaticLibrary("lib/util", Glob("lib/*.c"))

# net

env.Program("src/net", Glob("src/*.c") + ["lib/libutil.a",])

# convers

env.Program("convers/convers", ["convers/convers.c", "lib/libutil.a",])
env.Program("convers/conversd", ["convers/conversd.c", "lib/libutil.a",])

# util

env.Program("util/bridge", ["util/bridge.c", "lib/libutil.a",])
env.Program("util/cnet", ["util/cnet.c", "lib/libutil.a",])
env.Program("util/md5", ["util/md5drivr.c", "lib/libutil.a",])
env.Program("util/mkhostdb", ["util/mkhostdb.c", "lib/libutil.a",])
env.Program("util/path", ["util/path.c", "lib/libutil.a",])
env.Program("util/qth", ["util/qth.c", "lib/libutil.a",], LIBS=["m"])

# bbs

env.Program("bbs/bbs", ["bbs/bbs.c", "lib/libutil.a",])

# doc

for manual in env.Glob("doc/*.asciidoc", strings=True):
    html = manual[:-9] + ".html"
    env.Command(
        html,
        manual,
        "asciidoc -a toc -a numbered " + manual
    )

for picture in env.Glob("doc/*.gv", strings=True):
    png = picture[:-3] + ".png"
    env.Command(
        png,
        picture,
        "dot -Tpng $SOURCE -o $TARGET",
    )

def make_bbs_help_file(target, source, env):
    f = open("bbs/bbs.help", "w")
    for line in os.popen("w3m -dump doc/bbs.html", "r"):
        line = line.rstrip()
        m = re.match("^[0-9][0-9.]* (.*)", line)
        if m:
            word = m.group(1).split()[0]
            f.write("\n^" + word + "\n\n" + m.group(1) + "\n\n")
        else:
            f.write(line + "\n")
    f.close()
    return 0

env.Command(
    "bbs/bbs.help",
    "doc/bbs.html",
    make_bbs_help_file,
    )

# install

env.Install("/tcp", source = [
    "convers/conversd",
    "src/net",
    "util/bridge",
    "util/mkhostdb",
    ])

env.Install("/usr/local/bin", source = [
    "bbs/bbs",
    "convers/convers",
    "util/cnet",
    "util/md5",
    "util/path",
    "util/qth",
    ])

env.Install("/usr/local/lib", source = [
    "bbs/bbs.help",
    ])

for filename in os.listdir("examples"):
    if not os.path.exists("/tcp/" + filename):
        env.Command("/tcp/" + filename,
                    "examples/" + filename,
                    Copy('$TARGET', '$SOURCE'))

env.Command("/tcp/hostname.dir",
            ["/tcp/mkhostdb", "/tcp/domain.txt", "/tcp/domain.local", "/tcp/hosts", ],
            "/tcp/mkhostdb"
            )

# clean

env.Clean("/", FindInstalledFiles())

def same(filename1, filename2):
    try:
        if open(filename1).read() == open(filename2).read():
            return True
    except:
        pass
    return False

for filename in os.listdir("examples"):
    if same("/tcp/" + filename, "examples/" + filename):
        env.Clean("/", "/tcp/" + filename)

env.Clean("/", Glob("/tcp/hostname.*") + Glob("/tcp/hostaddr.*"))
