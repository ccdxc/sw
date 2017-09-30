# Flow generation configuration template.

meta:
    id: SESSION_ESP

proto: esp

entries:
    - entry:
        label: esp
        responder:
            spi : const/1
        initiator: 
            spi : const/0
