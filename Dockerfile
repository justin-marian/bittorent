FROM jokeswar/base-ctl

RUN apt-get update -yqq
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata
RUN apt-get install -yqq openmpi-bin
RUN apt-get install -yqq openmpi-common
RUN apt-get install -yqq openmpi-doc
RUN apt-get install -yqq libopenmpi-dev

COPY ./checker ${CHECKER_DATA_DIRECTORY}
