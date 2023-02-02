#! /bin/bash
imageFolder=$1
for files in ${imageFolder}/*;do 
   filename=${files##*/}
   command $($(cd .. && pwd)/bin/wsi-anon.out) ${files} -n anonymized_${filename%.*}
done