FROM ubuntu:latest

WORKDIR /app
ADD . /app

RUN apt-get update -y
RUN apt-get install python3 python3-pip git cmake -y
RUN pip3 install bs4 requests

RUN ["python3", "launch.py", "--f", "build"]

CMD ["python3", "launch.py", "--f", "run", "--r", "config/config.yaml", "config/environments.yaml"]
