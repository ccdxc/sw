#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file directory>"
    exit 1
fi
container=$1

while read -r line || [[ -n "$line" ]];
do
  pkg=$(echo $line | awk '{ print $2 }');
  echo "++--- generating docs for package ${pkg}";
  [[ -z "${pkg// }" ]] && continue;
  [[ "${pkg}" = "bookstore"  ]] && continue;
  docker run --user `id -u`:`id -g` --rm -v${PWD}:/import/src/github.com/pensando/sw -v${PWD}/bin/cbin:/import/bin -w /import/src/github.com/pensando/sw ${container} bash -c "/home/node/.npm-global/bin/spectacle -q -1 /import/src/github.com/pensando/sw/api/generated/${pkg}/swagger/external/svc_*.json -t /import/src/github.com/pensando/sw/api/docs/generated/${pkg}" 2&> /dev/null
done < ${PWD}/api/generated/pkgmanifest;
cp -r api/docs tools/docker-files/apigw;


rm -fr  bin/swagger2markup
mkdir -p bin/swagger2markup
cp api/swagger2markup/*.adoc bin/swagger2markup

for i in  $(ls api/generated/*/swagger/external/svc*swagger*)
do
    a=$(basename $i)
    name=${a%%.swagger.json} # something like svc_audit

    # set unique anchor so that each individual ascii doc has different tag so that the combined doc has unique references
    echo "swagger2markup.anchorPrefix=${name}" > bin/swagger2markup/config.properties

    docker run --user `id -u`:`id -g` --rm  -v $(pwd):/opt swagger2markup/swagger2markup convert -c /opt/bin/swagger2markup/config.properties -i /opt/${i}  -f /opt/bin/swagger2markup/${name}

    #fix the json section under Any from the generated asciidoctor so that document indents are proper
    perl -i  -p0e 's/\n= JSON\n\nThe JSON representation of an/\n==== JSON\n\nThe JSON representation of an/msg'  bin/swagger2markup/${name}.adoc

    # add to index so that pdf generation happens for this asciidoc
    echo "include::${name}.adoc"'[leveloffset=+1]' >> bin/swagger2markup/index.adoc
done

#finally generate pdf from the asciidoc files
docker run --user `id -u`:`id -g` --rm -v $(pwd):/documents/ asciidoctor/docker-asciidoctor asciidoctor-pdf   -o bin/venice_apidoc.pdf bin/swagger2markup/index.adoc
