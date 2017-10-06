# Flow generation configuration template.
meta:
    id: SESSION_ESP_PROXY

proto: esp 

entries:
    - entry:
        label: esp-proxy
        tracking: False
        timestamp: False
        responder: 
            spi : const/1
        initiator: 
            spi : const/0

