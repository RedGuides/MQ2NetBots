!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Netbots.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Netbots.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Netbots.dll"
	-@erase "$(OUTDIR)\MQ2Netbots.exp"
	-@erase "$(OUTDIR)\MQ2Netbots.lib"
	-@erase "$(OUTDIR)\MQ2Netbots.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Netbots.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Netbots.dll" /implib:"$(OUTDIR)\MQ2Netbots.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Netbots.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Netbots.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Netbots.dep")
!INCLUDE "MQ2Netbots.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Netbots.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Netbots.cpp

"$(INTDIR)\MQ2Netbots.obj" : $(SOURCE) "$(INTDIR)"

