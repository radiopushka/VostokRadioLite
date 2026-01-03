# FM/AM Broadcast Processor and Audio Maximizer for Edge Devices, specifically for Raspberry Pi.
## 94-100% single core utilization with the provided settings on the Raspberry Pi Zero W 
## For use with the RPI DAC+ module https://www.raspberrypi.com/products/dac-plus/


## Initial Setup
- **Download**: ```git clone https://github.com/radiopushka/VostokRadioLite```
- **Compilation**: ```cd VostokRadioLite``` then ```make```
- **Installation**: ```cp ./vtkradio /usr/bin``` or where ever you want to put it.
- **Configuration**: the sample_cfg file has all the configuration options available, pass it to vtkradio: ```vtkradio ./sample_cfg``` and it will show you the active configuration.

## Configuration file guide
- **The configuration file format expects no spaces between the key, the equals sign, and the value.**
- **Please follow the file format as specified, do not file bug reports if you are not sure the format is exactly correct**
### Audio Section
```
[audio]
iface=default
oface=default
```
- This section specifies what alsa interface to use for the playback (oface) and for recording (iface) for the real time processing
### MPX Section
```
[MPX]
pilot_amp=0.15
stereo_ratio=0.3
OF_bins=1
c1_mpx=1
c2_mpx=1
```
- This section contains the percentage of the pilot tone with respect to the signal(pilot_amp), the percentage of the 38khz modulation with respect to the signal(stereo_ratio).
- It also contains (OF_bins) which specifies how many frequencies from the top of the audio band (16khz) to limit the amplitude of waveforms that are non-orthogonal to the stereo pilot tone. This can improve stereo separation and reduce distortion but can also add distortion.
- The (c1_mpx) and (c2_mpx) fields specify whether to use the right or left channel or both for MPX instead of mono audio.



