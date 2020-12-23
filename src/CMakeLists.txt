add_executable(armadanes WIN32
    bus.cpp
    bus.h
    cpu.cpp
    cpu.h
    cpubus.cpp
    cpubus.h
    cpudefs.h
    cpuops.cpp
    d3d9renderer.cpp
    d3d9renderer.h
    main.cpp
    main.h
    mapper.cpp
    mapper.h
    mappernrom.cpp
    mappernrom.h
    ppu.cpp
    ppu.h
    ppubus.cpp
    ppubus.h
    rom.cpp
    rom.h
    system.cpp
    system.h

    armadanes.rc
    resource.h
)

target_link_libraries(armadanes d3d9)
target_compile_options(armadanes PRIVATE -Wall)