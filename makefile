ifeq ($(OS),Windows_NT)
  EXE =.exe
  OBJ =.obj
  CC  = cl
  CP  = copy
  RM  = del 2>nul
  CFLAGS = /nologo /D_CRT_SECURE_NO_WARNINGS /O2 /Fe:
  LFLAGS =
  BIN_DIR = c:\doc\bin
else
  EXE =
  OBJ = .o
  CC = cc
  CP = cp
  RM = rm -f
  CFLAGS = -O2 -o
  LFLAGS = -lm
  BIN_DIR = /data/doc/bin
endif

.PHONY : clean install

PROJ = $(notdir $(CURDIR))

$(PROJ)$(EXE) : $(PROJ).c
	$(CC) $(CFLAGS) $(PROJ)$(EXE) $(PROJ).c $(LFLAGS)

clean :
	@$(RM) $(PROJ)$(EXE) $(PROJ)$(OBJ) $(PROJ)_proto.h

install : $(PROJ)$(EXE)
	@$(CP) $(PROJ)$(EXE) $(BIN_DIR)
