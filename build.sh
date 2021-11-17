# Build binaries
docker build . -t wsi-anon-builder
docker run -v $(pwd):/code --rm wsi-anon-builder make
docker run -v $(pwd):/src  --rm emscripten/emsdk make wasm
docker run -v $(pwd):/code --rm wsi-anon-builder mv /code/bin/wsi-anon.out /code/bin/wsi-anon
docker run -v $(pwd):/code --rm wsi-anon-builder mv /code/bin/wsi-anon.out /code/bin/wsi-anon
docker run -v $(pwd):/code --rm wsi-anon-builder chmod -R 777 bin/ obj/

# Publish WASM binding as NPM package
docker run -v $(pwd):/code --rm --entrypoint bash node:16-slim -c "cd /code && npm publish --dry-run"
