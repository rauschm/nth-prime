ifeq ($(OS),Windows_NT)
  EXE =.exe
  OBJ =.obj
  CC  = cl
  CP  = copy
  RM  = del 2>nul
  CFLAGS = /nologo /D_CRT_SECURE_NO_WARNINGS /O2 /Fe:
  LFLAGS =
  BIN_DIR = c:\doc\bin
  VERIFY = verify.bat
else
  SHELL = /usr/bin/bash
  EXE =
  OBJ = .o
  CC  = cc
  CP  = cp
  RM  = rm -f
  CFLAGS = -O2 -o
  LFLAGS = -lm
  BIN_DIR = /data/doc/bin
  VERIFY = . verify.sh
endif

.PHONY : clean verify

PROJ = $(notdir $(CURDIR))

$(PROJ)$(EXE) : $(PROJ).c
	$(CC) $(CFLAGS) $(PROJ)$(EXE) $(PROJ).c $(LFLAGS)

clean :
	@$(RM) $(PROJ)$(EXE) $(PROJ)$(OBJ)

install : $(PROJ)$(EXE)
	@$(CP) $(PROJ)$(EXE) $(BIN_DIR)

verify :
	@$(VERIFY)
