CFLAGS += -DCS_PLATFORM=3 \
           -std=gnu99 \
           -ffunction-sections \
           -fdata-sections \
           -DMG_NO_BSD_SOCKETS \
           -DMG_DISABLE_FILESYSTEM \
           -DMG_ENABLE_SSL \
           -DMG_DISABLE_PFS \
           -D_POSIX_VERSION \
           -DRTOS_SDK -DMG_LWIP -DLWIP_TIMEVAL_PRIVATE=0 \
           -DMG_INTERNAL=

PROGRAM=http_post_mongoose
include ../../common.mk
