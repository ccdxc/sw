name: Host-Host
enabled: True
endpoint: 
  dst: host
  src: host

modules:
    - module:
        name    : PING TEST
        program : ping_test
    - module:
        name    : HTTP TEST
        program : http_test