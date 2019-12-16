import fileinput
import os
filepath = './minio/VERSIONS-VENICE'
cnt = 0
new_version = ""

# Utility functino to upgrade venice_web_app_framework version
# in minio/VERSIONS file in place
# Then it will upload the asset to minio

if __name__ == "__main__":
    for line in fileinput.input(filepath, inplace=True):
        cnt += 1
        line = line.rstrip('\r\n')
        if cnt == 1:
            print(line)
            continue
        key = (line.strip().split(" "))[0]
        version = (line.strip().split(" "))[1]
        if key == 'venice_web_app_framework':
            version_digits = map(int, version.split("."))
            version_digits[2] = version_digits[2] + 1
            new_version = ".".join(map(str, version_digits))
            print("{} {}".format(key, new_version))
        else:
            print(line)
        
    os.system('asset-upload venice_web_app_framework {} asset.tgz'.format(new_version))

