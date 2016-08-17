CFLAGS += -DCS_PLATFORM=3 \
           -std=gnu99 \
           -ffunction-sections \
           -fdata-sections \
           -DMG_NO_BSD_SOCKETS \
           -DMG_DISABLE_FILESYSTEM \
           -DMG_DISABLE_HTTP_KEEP_ALIVE \
           -DMG_DISABLE_MQTT \
           -DMG_DISABLE_SHA1 \
           -DMG_DISABLE_MD5 \
           -DMG_DISABLE_HTTP_DIGEST_AUTH \
           -DRTOS_SDK -DMG_LWIP -DLWIP_TIMEVAL_PRIVATE=0 \
           -DMG_INTERNAL=

PROGRAM=restful_server_mongoose
include ../../common.mk
