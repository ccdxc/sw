#!/bin/sh

CONFIG=/sysconfig/config0
CONSOLE_CONFIG=${CONFIG}/.console

enable() {
    touch ${CONSOLE_CONFIG}
    CONSOLE_PID=$(pidof console)
    if [ ! -z "${CONSOLE_PID}" ]; then
        kill -SIGTERM ${CONSOLE_PID}
    fi
}

disable() {
    rm -f ${CONSOLE_CONFIG}
    echo "Please log out from the console"
}

case "$1" in
    enable)
        enable
        ;;
    disable)
        disable
        ;;
    *)
        echo "Usage: $0 {enable|disable}"
        exit 1
esac

exit $?
