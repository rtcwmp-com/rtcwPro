
What is docker and why are we using it?
---------------------------------------
The purpose of the docker files here are to build, test, and run rtcwPro in a reproducible manner. 

Docker is a container system where docker images define a specific OS configuration and instructions for what the container does when it is run. Once the image is built, it does not change. Each time the image is run, it starts fresh as if the OS was just installed.

Often it is desirable to share files between the host and the container. There's a few methods available to accomplish this, such as providing a location on the host's filesystem to be available to the container. 

For example, the compile-develop image builds the rtcwPro github files inside a container and shares the build artifacts with the host system. The compile image only knows to pull the latest files from the github repository, then execute the build process. Once it has finished the task, the image exits, and the build files are now available at the location shared with the container. 

Similarly for the server-dev image, we're sharing the build artifacts on the host system with the container to use at runtime. This allows for more rapid iteration on making changes and running the containerized server with them. 

Ultimately, the main benefit of using docker is standardization and automation. 

Overview of the docker files:
-----------------------------

- Basegame 
  - The base rtcw pk3s from Steam
  - Requires a steam account that purchased rtcw to build
  - Only needs to be built once


- Server-dev
  - Uses basegame's files 
  - Uses symlinks for all of the rtcwPro files to a mounted location provided at runtime
  - Based on msh100's docker rtcw server
  - Runs entrypoint.sh on startup (the version of it when the image is built)


- Compile-develop
  - Pulls the latest files from rtcwPro's develop branch on github and builds them
  - Build artifacts are moved to a location with the same file hierarchy used by Server-dev
  - Runs make-develop.sh on startup


- Compile-master
  - Same as Compile-develop but uses the master branch
- Compile-test
  - Same as Compile-Develop but uses the test branch


Getting started / Building the images
---------------
To get started, you'll need docker. It's possible to run Linux images from Windows docker, but for now I'll only describe the process of using docker within linux since we need to build the images too. 

If you don't have access to a Linux OS, Windows Subsystem for Linux (WSL) makes it very easy. From a command prompt or powershell window simply run `wsl --install` to install a basic Ubuntu OS that Windows can natively access. See more here: https://learn.microsoft.com/en-us/windows/wsl/install 

The rest of the instructions will be run from a Linux terminal. 

We'll need docker and git at a minimum. `sudo apt-get install docker git`

By default, `git clone <url>` will check out the master branch, but for development we'll use the develop branch of rtcwPro.

Check out the rtcwPro develop branch: `git clone -b feature/dockerbuild https://github.com/rtcwmp-com/rtcwPro`

Change directory to the rtcwPro docker-scripts area: `cd rtcwPro/docker-scripts`

To build the docker images there's several scripts to assist. To make the scripts executable: `chmod +x *.sh`

The server depends on the basegame, so build that one first. `./build-image-basegame.sh`
- It will prompt for your steam username, password, and optional guardcode. 
- If you have Steam multi-factor authentication enabled, you'll have to run the basegame script twice.
  - First without the guard code. You'll get an email or text message with the code. 
  - Then with the guard code you received. 

Build the dev server image: `./build-image-devserver.sh`

Build the compile-develop image: `./build-image-compile-develop.sh`

Now we can build the rtcwPro files: `./run-compile-dev.sh`

To see the files that were built: `ls ../build/*`


Running the server
------------------

First let's examine the file that runs the dev server: `run-devserver.sh`
```bash
#!/bin/bash
addrs=(`hostname -I`)
docker run --mount type=bind,src=`pwd`/../build,dst=/home/game/dev   -p "${addrs[0]}:27960:27960/udp"   -e "MAPS=adlernest_b3:te_escape2:te_frostbite"   -e "PASSWORD=war"   -e "REFEREEPASSWORD=pass123" -e "SERVERCONF=comp" rtcwpro/server-dev:1.0
```
- The addrs variable will contain the IP address the server will use. You can see this by running `hostname -I` yourself. 

- The --mount argument tells docker to take the the build directory, up one level from the current working directory, and mount it in the image at the location /home/game/dev. 

- The -p argument tells docker what ports to publish to the host system. 

- The -e arguments are environment variables that are made available at runtime to the script `dockerfiles/scripts/entrypoint.sh`

- To modify the file a basic text editor installed by default can be used: `nano run-devserver.sh`
- Further detail on the environment variables here: https://github.com/msh100/rtcw/blob/master/README.md

Now we can run the dedicated server that uses our development files: `./run-devserver.sh`

To view running containers: `docker ps`

To attach a shell to a running container (to interact with it): `docker exec -i -t <container id from docker ps> /bin/bash`

To debug the server from the attached shell (the server is always PID 1): `gdb -p 1` further detail on GDB is outside the scope of this document. 

To stop a container: `docker stop <container id or container pseudo name>`