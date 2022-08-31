#!/bin/bash
string=$(cat /etc/passwd | grep home)
array=(${string//:/ })

if [ ! -d /home/$array/.oh-my-zsh ]; then
    sudo cp -r /root/.oh-my-zsh /home/$array/
    sudo cp /root/.zshrc /home/$array/.zshrc
    sudo cp -r /root/.jupyter /home/$array
    sudo chown -R $array:$array /home/$array
    sudo echo "export PATH=/usr/local/cuda-11/bin;\$PATH" >> /etc/bashrc
    sudo echo "export LD_LIBRARY_PATH=/usr/local/cuda-11/lib64:\$LD_LIBRARY_PATH" >> /etc/bashrc
    echo "c.NotebookApp.notebook_dir = '/home/$array/workspace'" >> /home/$array/.jupyter/jupyter_notebook_config.py
fi
sudo service ssh start 
jupyter notebook 
