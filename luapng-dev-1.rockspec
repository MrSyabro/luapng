package = "luapng"
version = "dev-1"
source = {
   url = "git+https://git@github.com/MrSyabro/luapng.git",
   branch = "master",
}
description = {
   homepage = "https://github.com/MrSyabro/luapng",
   license = "MIT/X11",
   maintainer = "MrSyabro",
}
dependencies = {
   "lua >= 5.2"
}

build = {
   type = "builtin",
   modules = {
      luapng = {
        sources = { "src/luapng.c", },
        libraries = {"png"},
      }
   },
}