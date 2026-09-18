source [find interface/kitprog3.cfg]

transport select swd


source [find target/traveo2_1m_a0.cfg]
${TARGET}.cm0 configure -rtos auto -rtos-wipe-on-reset-halt 1
traveo2 sflash_restrictions 1
