@echo off
echo Starting WSL build...
wsl -d Ubuntu bash -c "cd ~/ddnet-build && rm -rf * && cmake /mnt/c/Users/5year/OneDrive/Desktop/ddnet-master -DCMAKE_BUILD_TYPE=Release -DSERVER=ON -DCLIENT=OFF -DTOOLS=OFF -DVIDEORECORDER=OFF -DAUTOUPDATE=OFF && make DDNet-Server -j4 && cp DDNet-Server /mnt/c/Users/5year/OneDrive/Desktop/ddnet-master/build-wsl/"
echo Build complete!
pause
