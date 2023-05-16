FROM registry.gitlab.com/empaia/integration/ci-docker-images/test-runner:0.1.78@sha256:7cbf05b3903c4989b84234713998e3ac64bc55c5dbf027b3042397a3522e754a

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get install --no-install-recommends -y \
  build-essential python3-dev python3-pip openslide-tools python3-openslide

RUN pip install openslide-python

COPY . /wsi-anon

WORKDIR /wsi-anon

RUN make

RUN pip3 install -r wrapper/python/test/requirements.txt

ENTRYPOINT ["tail", "-f", "/dev/null"]
