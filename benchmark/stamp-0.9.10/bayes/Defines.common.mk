# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.

hostname := $(shell hostname)

PROG := bayes

SRCS += \
	adtree.c \
	bayes.c \
	data.c \
	learner.c \
	net.c \
	sort.c \
	$(LIB)/bitmap.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DLEARNER_TRY_REMOVE
CFLAGS += -DLEARNER_TRY_REVERSE

RUNPARAMS := -v32 -r4096 -n10 -p40 -i2 -e8 -s1

.PHONY:	run1 run2 run4 run6 run8 run12 run16 run32 run64 run128 run-16 run-32 run-64

run1:
	$(PROGRAM) $(RUNPARAMS) -t1

run2:
	$(PROGRAM) $(RUNPARAMS) -t2

run4:
	$(PROGRAM) $(RUNPARAMS) -t4

run6:
	$(PROGRAM) $(RUNPARAMS) -t6

run8:
	$(PROGRAM) $(RUNPARAMS) -t8

run12:
	$(PROGRAM) $(RUNPARAMS) -t12

run16:
	$(PROGRAM) $(RUNPARAMS) -t16

run32:
	$(PROGRAM) $(RUNPARAMS) -t32

run64:
	$(PROGRAM) $(RUNPARAMS) -t64

run128:
	$(PROGRAM) $(RUNPARAMS) -t128

run-16:
	$(PROGRAM) $(RUNPARAMS) -t-16

run-32:
	$(PROGRAM) $(RUNPARAMS) -t-32

run-64:
	$(PROGRAM) $(RUNPARAMS) -t-64

# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
