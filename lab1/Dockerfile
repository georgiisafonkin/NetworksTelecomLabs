FROM debian:latest

WORKDIR /app

COPY . .

RUN apt-get update -y && apt-get upgrade -y && apt-get install gcc -y
RUN gcc -o main main.c -pthread

CMD ["./main", "239.255.255.250", "224.0.0.4", "8080"]