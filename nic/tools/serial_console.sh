#!/bin/sh

CONFIG=/sysconfig/config0
CONSOLE_CONFIG_DEPRECATED=${CONFIG}/.console
SYSTEM_CONFIG_FILE=${CONFIG}/system-config.json

enable() {
    if [ ! -f $SYSTEM_CONFIG_FILE ]; then
        jq -n --arg keyval enable '{ "console": $keyval }' > \
            $SYSTEM_CONFIG_FILE
    else
        # jq can't edit file in place, write to temp file and move it over. 
        jq '. + { "console": "enable" }' $SYSTEM_CONFIG_FILE > \
            ${SYSTEM_CONFIG_FILE}_console
        mv ${SYSTEM_CONFIG_FILE}_console $SYSTEM_CONFIG_FILE
    fi

    CONSOLE_PID=$(pidof console)
    if [ ! -z "${CONSOLE_PID}" ]; then
        kill -SIGTERM ${CONSOLE_PID}
    fi
}

disable() {
    # Remove both old way of configuration (.console file) and
    # the current form of json key value in CONSOLE_CONFIG file.
    rm -f ${CONSOLE_CONFIG_DEPRECATED}
    if [ -f $SYSTEM_CONFIG_FILE ]; then
        jq 'del(.console)' ${SYSTEM_CONFIG_FILE} > \
            ${SYSTEM_CONFIG_FILE}_console
        mv ${SYSTEM_CONFIG_FILE}_console $SYSTEM_CONFIG_FILE
    fi
    
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
