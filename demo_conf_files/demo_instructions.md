# O-RAN 7.2 Split — Two-Host Setup

Host 1: O-RU + UE

Host 2: O-DU + 5G Core

---

## Configuration Files

Host 1:
- RU conf: `~/openairinterface5g/targets/PROJECTS/GENERIC-NR-5GC/CONF/ru.band77.mu1.106rb.2x2.new.conf`
- UE conf: `~/openairinterface5g/targets/PROJECTS/GENERIC-NR-5GC/CONF/nrue.conf`
- PTP config: `/etc/ptp4l.conf`

Host 2:
- DU conf: `~/openairinterface5g/targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.band77.mu1.106rb.fhi.2x2.new.conf`
- PTP config: `/etc/ptp4l.conf`

---

## Build Steps

### Host 1 (O-RU + UE): CMake Configure and Build

```bash
cd ~/openairinterface5g/cmake_targets/build

cmake ../.. \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DOAI_RU_FRONTHAUL=ON

ninja -j$(nproc) \
    nr-oru \
    nr-uesoftmodem \
    vrtsim \
    params_libconfig
```

### Host 2 (O-DU): Build DPDK 23.11

```bash
cd ~
pip3 install --user meson pyelftools
wget https://fast.dpdk.org/rel/dpdk-23.11.tar.xz
tar xf dpdk-23.11.tar.xz
cd dpdk-23.11
meson setup \
    --prefix=~/dpdk-inst \
    -Dplatform=native \
    -Denable_kmods=false \
    -Dtests=false \
    build
cd build
ninja
ninja install
```

### Host 2 (O-DU): Build xRAN K Release v11.1.1

```bash
cd ~
git clone https://github.com/openairinterface/o-du-phy.git \
    --branch 11.1.1 --depth 1 xran

mkdir -p ~/dpdk-inst/x86_64-native-linux-gcc
ln -sfn ~/dpdk-inst/include ~/dpdk-inst/x86_64-native-linux-gcc/include
ln -sfn ~/dpdk-inst/lib/x86_64-linux-gnu ~/dpdk-inst/x86_64-native-linux-gcc/lib

cd ~/xran/fhi_lib/lib
PKG_CONFIG_PATH=~/dpdk-inst/lib/x86_64-linux-gnu/pkgconfig \
    WIRELESS_SDK_TOOLCHAIN=gcc \
    TARGET_PROCESSOR=x86_64 \
    RTE_SDK=~/dpdk-inst \
    RTE_TARGET=x86_64-native-linux-gcc \
    XRAN_DIR=.. \
    make XRAN_LIB_SO=1
```

### Host 2 (O-DU): CMake Configure and Build

```bash
cd ~/openairinterface5g/cmake_targets/build

PKG_CONFIG_PATH=~/dpdk-inst/lib/x86_64-linux-gnu/pkgconfig \
cmake ../.. \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DOAI_RU_FRONTHAUL=ON \
    -DOAI_FHI72=ON \
    -Dxran_LOCATION=~/xran/fhi_lib/lib \
    -DCMAKE_EXE_LINKER_FLAGS="-L~/dpdk-inst/lib/x86_64-linux-gnu \
        -Wl,-rpath,~/dpdk-inst/lib/x86_64-linux-gnu"

ninja -j$(nproc) \
    nr-softmodem \
    oran_fhlib_5g \
    params_libconfig
```

---

## Steps to Run

### 1. SR-IOV Setup (both hosts, after every reboot)

On Host 1:
```bash
sudo /etc/ran_development_templates/setup_sriov_vfs.sh
sudo setup_sriov_ru_vlan3.sh
```

On Host 2:
```bash
sudo /etc/ran_development_templates/setup_sriov_vfs.sh
sudo setup_sriov_du_vlan3.sh
```

### 2. PTP Synchronization

Restart ptp4l if needed:

On Host 1:
```bash
sudo systemctl restart ptp4l
```

On Host 2:
```bash
sudo systemctl restart ptp4l
```

### 3. Start 5G Core Network (Host 2)

```bash
cd ~/oai-cn5g-fed/docker-compose
docker compose -f docker-compose-mini-nonrf.yaml up -d
```

### 4. Start O-DU (Host 2)

```bash
cd ~/openairinterface5g/cmake_targets/build
sudo LD_LIBRARY_PATH=.:~/dpdk-inst/lib/x86_64-linux-gnu \
    ./nr-softmodem \
    -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.band77.mu1.106rb.fhi.2x2.new.conf \
    --thread-pool 16,17,18,19
```

### 5. Start O-RU (Host 1)

```bash
cd ~/openairinterface5g/cmake_targets/build
sudo taskset -c 5,6,7,8,9,15 env LD_LIBRARY_PATH=.:~/dpdk-inst/lib/x86_64-linux-gnu \
    ./nr-oru \
    -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/ru.band77.mu1.106rb.2x2.new.conf \
    --vrtsim.role server
```

### 6. Start UE (Host 1)

```bash
cd ~/openairinterface5g/cmake_targets/build
sudo taskset -c 12-14,16-19 \
    ./nr-uesoftmodem \
    -C 4049760000 -r 106 --numerology 1 --ssb 516 \
    --device.name vrtsim --vrtsim.role client \
    -O ../targets/PROJECTS/GENERIC-NR-5GC/CONF/nrue.conf
```

### 7. Traffic Test

On Host 2, get ext-DN IP:
```bash
docker inspect oai-ext-dn | grep IPAddress
```

On Host 2, start iperf server:
```bash
docker exec -it oai-ext-dn iperf3 -s
```

On Host 1, check UE IP:
```bash
ip addr show oaitun_ue1
```

On Host 1, ping:
```bash
ping 192.168.70.135 -I oaitun_ue1 -c 5
```

On Host 1, iperf:
```bash
iperf3 -c 192.168.70.135 -B <ue-ip> -t 100
```
