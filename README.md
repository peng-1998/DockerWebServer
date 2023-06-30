# DockerWebServer

Docker Web Server is a app design for managing GPU server in Lab.

## Dependencies

We use Python flask as backend framework.
To fetch the status of docker and GPU, we use docker and pynvml.
This project also relies on `docker` and `nvidia-smi` for data, so please make sure these two commands are available.

We use react to build our web UI.

**Check out our WebUI Repo** https://github.com/austiecodes/dws-ui


### Software



## Permissions

If you are unable to run the program as root.
  + The project uses a linux user and password to log in, you need to read the /etc/shadow file to verify the password, make sure the running user has read access when running
  + The program needs access to the docker access, please make sure the run user is in the docker user group

## Mirroring

We have designed a special configuration for the container to ensure that the container has the same privileges as the user on the host, so there are requirements for the mirroring environment.
The main requirement is that the sudo package is installed in the container.

Then, the four files `/etc/passwd`, `/etc/group`,` /etc/shadow`, `/etc/sudoers` need to be extracted from the image and placed in `data/image/{image name}/`. Also you need to write the `start.sh` script, which creates the `/home/{user}` directory and changes the creator to the specified user using the shown command, and executes a running process at the end to keep the container running (e.g., `while true; do sleep 10000000; done`). You can refer to` data/image/conda:py38_492/start.sh`

The image build can be found in `dockerfile/Dockerfile.servermanager_pytorch`
