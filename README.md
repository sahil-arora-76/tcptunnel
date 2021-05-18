A simple tcp tunnel on c using sockets 
Right now it only supports linux systems

# build BY MAKE
    
``` 
mkdir build 
make 
cd build 
./tunnel.o <localport> <remoteport> <remotehost>
```
# OR (recommended)

``` 
chmod +x ./tunnel.sh
./tunnel.sh 
cd build 
./tunnel.o <localport> <remoteport> <remotehost>
``` 


