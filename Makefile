# Exported from https://tiplanet.org/pb/ on Wed Apr 26 16:52:35 2023 (CEST)

# ----------------------------
# Program Options
# ----------------------------

NAME         ?= UNIVERSE
ICON         ?= icon.png
DESCRIPTION  ?= ""
COMPRESSED   ?= YES
ARCHIVED     ?= YES

# ----------------------------
# Compile Options
# ----------------------------

CFLAGS   ?= -Oz -W -Wall -Wextra -Wwrite-strings
CXXFLAGS ?= -Oz -W -Wall -Wextra -Wwrite-strings

# ----------------------------

include $(shell cedev-config --makefile)
