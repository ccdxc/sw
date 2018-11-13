# To make this script idempotent, we first decompress all gz files
# and then delete all gz files
cd dist
find . -name "*.gz" -type f |  while read -r fileGz
do
  gunzip $fileGz
done
find . -name "*.gz" -type f -delete

# Following comment is taken from NYTimes/gziphandler
# DefaultMinSize is the default minimum size until we enable gzip compression.
# 1500 bytes is the MTU size for the internet since that is the largest size allowed at the network layer.
# If you take a file that is 1300 bytes and compress it to 800 bytes, it’s still transmitted in that same 1500 byte packet regardless, so you’ve gained nothing.
# That being the case, you should restrict the gzip compression to files with a size greater than a single packet, 1400 bytes (1.4KB) is a safe value.
defaultMinSize=1400

# We compress the following types: js css html json ico eot otf ttf
# NOTE: If adding any other types, make sure to check that gzipserver.go supports the extra types.
find . -size +${defaultMinSize}c -type f \( -iname \*.js -o -iname \*.css -o -iname \*.html -o -iname \*.json -o -iname \*.ico -o -iname \*.eot -o -iname \*.otf -o -iname \*.ttf  \) -exec gzip --best --keep {} \;
find . -name "*.gz" -type f |  while read -r fileGz
# we make sure we didnt increase any file sizes by compressing, and delete the larger file
do
  # Removing extention 
  len=${#fileGz}-3
  file=${fileGz:0:$len}
  # Comparing sizes and keeping smaller file
  actualsize=$(wc -c <"$file")
  gzipsize=$(wc -c <"$fileGz")
  if [ $gzipsize -lt $actualsize ]
  then
    rm $file
  else
    rm $fileGz
  fi
done