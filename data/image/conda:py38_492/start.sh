#!/bin/bash
string=$(cat /etc/passwd | grep home)
array=(${string//:/ })

if [ ! -d /home/$array/.oh-my-zsh ]; then
    sudo cp -r /root/.oh-my-zsh /home/$array/
    sudo cp /root/.zshrc /home/$array/.zshrc
    sudo chown -R $array:$array /home/$array
fi
sudo service ssh start 
while true; do sleep 10000000; done

