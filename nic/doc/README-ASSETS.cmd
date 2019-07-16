To add new files to minio:

Bump up the version number in minio/VERSIONS
Add the new files to the ".txt" files in minio/
Add new files/dir under .gitignore (sw/.gitignore or nic/sdk/.gitignore)
From sw dir, run "make UPLOAD=1 create-assets"
Make sure no errors
