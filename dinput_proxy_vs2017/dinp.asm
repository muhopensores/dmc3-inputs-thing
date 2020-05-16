.686
.MODEL FLAT, C
.STACK
.CODE
;-----------External usage--------------------------
EXTRN  DirectInput8Create : dword;
EXTRN  DllCanUnloadNow : dword;
EXTRN  DllGetClassObject : dword;
EXTRN  DllRegisterServer : dword;
EXTRN  DllUnregisterServer: dword;
;-----------Function definitions--------------------
_DirectInput8Create PROC
	jmp dword ptr [DirectInput8Create]
_DirectInput8Create ENDP

_DllCanUnloadNow PROC
	jmp dword ptr [DllCanUnloadNow]
_DllCanUnloadNow ENDP

_DllGetClassObject PROC
	jmp dword ptr [DllGetClassObject]
_DllGetClassObject ENDP

_DllRegisterServer PROC
	jmp dword ptr [DllRegisterServer]
_DllRegisterServer ENDP

_DllUnregisterServer PROC
	jmp dword ptr [DllUnregisterServer]
_DllUnregisterServer ENDP

END