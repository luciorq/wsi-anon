FROM python:3.9 AS wsi_service_dev

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get install --no-install-recommends -y \
  build-essential python3-openslide

RUN mkdir /openslide_deps
RUN cp /usr/lib/x86_64-linux-gnu/libopenslide.so.0 /openslide_deps
RUN ldd /usr/lib/x86_64-linux-gnu/libopenslide.so.0 \
  | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' /openslide_deps

COPY . /wsi-anon

WORKDIR /wsi-anon

RUN make

RUN pip3 install -r wrapper/python/test/requirements.txt
