foreach(prog image2fits imagecalc imageregrid imageslice)
    add_executable (${prog}  ${prog}.cc)
    target_link_libraries (${prog} casa_images ${CASACORE_ARCH_LIBS})
    install(TARGETS ${prog} DESTINATION bin)
endforeach(prog image2fits imagecalc imageregrid imageslice)
