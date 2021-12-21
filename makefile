ifeq ($(OS),Windows_NT)
  EXE =.exe
  OBJ =.obj
  CC  = cl /nologo /D_CRT_SECURE_NO_WARNINGS /O2 /Fe:
  CP  = copy
  RM  = del 2>nul
  BIN_DIR = c:\doc\bin
else
  EXE =
  OBJ = .o
  CC = cc -O2 -lm -o
  CP = cp
  RM = rm -f
  BIN_DIR = /data/doc/bin
endif

.PHONY : clean install

PROJ = $(notdir $(CURDIR))

$(PROJ)$(EXE) : $(PROJ).c
	$(CC) $(PROJ)$(EXE) $(PROJ).c

clean :
	@$(RM) $(PROJ)$(EXE) $(PROJ)$(OBJ) $(PROJ)_proto.h

install : $(PROJ)$(EXE)
	@$(CP) $(PROJ)$(EXE) $(BIN_DIR)
