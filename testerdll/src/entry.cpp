#include "include/flogger.hpp"

void getInfo() {
    auto console = std::make_unique<FLog>(nullptr);

    uintptr_t imageBase = std::bit_cast<uintptr_t>(GetModuleHandleA(nullptr));

    PIMAGE_DOS_HEADER pDosHeader           = std::bit_cast<PIMAGE_DOS_HEADER>(imageBase);
    PIMAGE_NT_HEADERS pNtHeader            = std::bit_cast<PIMAGE_NT_HEADERS>(imageBase + pDosHeader->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeader->OptionalHeader;

    uintptr_t iatAddr = imageBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;

    while (!GetAsyncKeyState(VK_END)) {
        console->clear();
        console->log("image base  -> 0x%X\n", imageBase);
        console->log("iat -> 0x%X\n\n", iatAddr);

        PIMAGE_IMPORT_DESCRIPTOR importDesc =
            std::bit_cast<PIMAGE_IMPORT_DESCRIPTOR>(pOptionalHeader->ImageBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        for (; importDesc->Name != 0; importDesc++) {
            const char* moduleName = std::bit_cast<const char*>(pOptionalHeader->ImageBase + importDesc->Name);
            console->log(LogLevel::lightcyan, "\n[%s]\n", moduleName);

            PIMAGE_THUNK_DATA nameThunk = std::bit_cast<PIMAGE_THUNK_DATA>(pOptionalHeader->ImageBase + importDesc->OriginalFirstThunk);

            for (; nameThunk->u1.AddressOfData != 0; nameThunk++) {
                if (nameThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
                    // imported by ordinal
                    console->log("  ordinal: %d\n", IMAGE_ORDINAL32(nameThunk->u1.Ordinal));
                } else {
                    // imported by name
                    auto importByName = std::bit_cast<PIMAGE_IMPORT_BY_NAME>(pOptionalHeader->ImageBase + nameThunk->u1.AddressOfData);
                    console->log("  name:    %s\n", importByName->Name);
                }
            }
        }

        system("pause");
    }

    console->clear();
}

void initialize(HINSTANCE hinstDLL) {
    if (MessageBoxA(nullptr, "Want to allocate console?", "console", MB_YESNO) == IDYES) getInfo();

    FreeLibraryAndExitThread(hinstDLL, 0);
}

bool __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpReserved*/) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)initialize, hinstDLL, 0, nullptr);
    }

    return true;
}
