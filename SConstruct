# Author: Dieter Deyke <dieter.deyke@gmail.com>
# Time-stamp: <2015-11-23 20:55:55 deyke>

import glob
import os
import stat

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

for manual in ("bbs", "bridge", "smack", "wampes", ):
    env.Command(
        "doc/" + manual + ".mm",
        [ sorted(glob.glob(os.path.join("manuals", manual, f.strip()))) for f in open(os.path.join("manuals", manual, "filelist")).readlines() ],
        "cat $SOURCES > $TARGET"
    )
    env.Command(
        "doc/" + manual + ".html",
        [ "doc/" + manual + ".mm", "tools/mm2html.py", ],
        "tools/mm2html.py doc/" + manual + ".mm > $TARGET"
    )

# install

env.Install("/tcp", source = [
    "convers/conversd",
    "src/net",
    "util/bridge",
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

env.Clean("/", FindInstalledFiles())
