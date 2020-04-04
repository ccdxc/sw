---
id: updateservercertificate
---


# Update Server Certificate

The PSM cluster by default creates a self-signed certificate during creation of the cluster.  Users are allowed to update the PSM cluster to use their own certificate and key.  This will remove the warning page when accessing the PSM cluster login page.

For the encoded key format, RSA and ECDSA keys are supported.  To configure this change, click "Admin" --> "Server Certificate".  Enter the key and certificate in PEM format and then click on "Upload" to apply the change.
