# Standalone CYT2B63CAE CM0+ bring-up; no production FOC sources are linked.
MTB_TYPE=COMBINED
TARGET=IFL-2B6-3IN1-BLINK
APPNAME=ifl_2b6_led_blinky
TOOLCHAIN=GCC_ARM
CONFIG=Debug
CORE=CM0P
CORE_NAME=CM0P_0

CY_TOOLS_PATHS?=C:/Infineon/Tools/ModusToolbox/tools_3.9
CY_TOOLS_DIR?=$(CY_TOOLS_PATHS)
CY_COMPILER_GCC_ARM_DIR?=C:/Users/ASUS/Infineon/Tools/mtb-gcc-arm-eabi/14.2.1/gcc
PDL_DIR?=C:/Users/ASUS/.modustoolbox/global/mtb-pdl-cat1/release-v3.24.0
CY_GETLIBS_SHARED_PATH=vendor
CY_GETLIBS_PATH=libs
CY_GETLIBS_DEPS_PATH=deps

SEARCH_TARGET_IFL-2B6-3IN1-BLINK=bsps/TARGET_IFL-2B6-3IN1-BLINK
SEARCH_core-make=vendor/core-make-release-v3.10.0
SEARCH_recipe-make-cat1a=vendor/recipe-make-cat1a-release-v2.8.0
SEARCH_core-lib=vendor/core-lib-release-v1.8.0
SEARCH_mtb-pdl-cat1=$(PDL_DIR)
SEARCH=$(PDL_DIR) $(SEARCH_core-lib)
INCLUDES=../../CMSIS/Include
COMPONENTS=CAT1 CAT1A TVIIBE1M
DISABLE_COMPONENTS=CM0P_SLEEP
LINKER_SCRIPT=bsps/TARGET_IFL-2B6-3IN1-BLINK/COMPONENT_CM0P/TOOLCHAIN_GCC_ARM/linker.ld
SKIP_CODE_GEN=true
CY_IGNORE=vendor/core-make-release-v3.10.0 vendor/recipe-make-cat1a-release-v2.8.0

# Keep dependency management strictly separate from the existing SDK repos.
ifneq ($(filter getlibs,$(MAKECMDGOALS)),)
$(error This example uses pinned local dependencies. Do not run getlibs)
endif

include $(CY_TOOLS_DIR)/make/start.mk
