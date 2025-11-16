<div align="center">

  ![doba](../../resources/doba_logo.png)

</div>

# OVERVIEW

# BUILD

To build a **Debug** runtime configuration use:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)


For <strong>Release</strong> configuration use:<br><br>
<pre style="
  color: rgb(120, 230, 115);
  background-color: rgba(32, 32, 32, 1.0);">
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
</pre>
Where,<br><br>
&nbsp;&nbsp;<em><vcpkg-installation-path></em> is the folder where you installed <em>vcpkg</em> (see <em>components</em> section).<br>
</span>

<h2 style="font-size:48px;">
  <strong>RUN<strong>
</h2>

<span style="font-family:'Courier New', monospace; font-size:14px">
After building the binary (and copying the required configuration files) you should get something like the following:<br><br>
.<br>
├─<strong>gateway</strong><br>
│&nbsp;&nbsp;└─build<br>
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├─gateway        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Microservice binary file.<br>
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;├─ca-root.pem    &nbsp;&nbsp;&nbsp;&nbsp;Needed certificate in order to consume Azure based resources (you can find it in the <em>/setup/microsoft/</em> folder).<br>
│&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;└─registry.json  &nbsp;&nbsp;Needed configuration file if using locally provided registry services (automatically copied by default from <em>/libraries/registry/</em> folder).<br>
.<br>
</span>

<h2 style="font-size:48px;">
  <strong>DOCKER<strong>
</h2>

<h2 style="font-size:32px;">
  &#9655; <strong>Linux (Debian)</strong>
</h2>

<span style="font-family:'Courier New', monospace; font-size:14px">
To setup <em>docker</em> just execute the following commands in a terminal:<br><br>
<pre style="
  color: rgb(120, 230, 115);
  background-color: rgba(32, 32, 32, 1.0);">
sudo rm -f /etc/apt/sources.list.d/docker.list
sudo rm -f /etc/apt/keyrings/docker.gpg
sudo apt update
sudo apt install -y ca-certificates curl gnupg lsb-release
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/debian/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg 
sudo chmod a+r /etc/apt/keyrings/docker.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/debian bookworm stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
</pre>
To build the <em>docker</em> image just execute the following commands in a terminal:<br><br>
<pre style="
  color: rgb(120, 230, 115);
  background-color: rgba(32, 32, 32, 1.0);">
docker build -t gateway:latest .
</pre>
To run the <em>docker</em> image just execute the following commands in a terminal:<br><br>
<pre style="
  color: rgb(120, 230, 115);
  background-color: rgba(32, 32, 32, 1.0);">
docker run --rm -p 10000:10000 gateway:latest
</pre>
</span>
