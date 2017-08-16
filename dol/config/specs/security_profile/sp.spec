# Configuration Template.
meta:
    id: SECURITY_PROFILES

profiles:
    - id: SEC_PROF_ACTIVE 
      fields:

    - id: SEC_PROF_DEFAULT
      fields:

    - id: SEC_PROF_IP_RSVD_FLAG_ALLOW
      fields:
        ip_rsvd_flags_action: ALLOW

    - id: SEC_PROF_IP_RSVD_FLAG_DROP
      fields:
        ip_rsvd_flags_action: DROP

    - id: SEC_PROF_IP_RSVD_FLAG_EDIT
      fields:
        ip_rsvd_flags_action: EDIT
