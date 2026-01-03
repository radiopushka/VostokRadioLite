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
### Limiter Section
```
[limiter]
pre_amp=0.7
lookahead=3
post_amp=1
limit=3.5e+9
save_harmonics=0.8
bins=9
```
- (pre_amp) specifies the pre-amp constant of the incoming audio before the agc(the agc comes before the complex limiter)
- (lookahead) specifies how many samples to process for each FFT coefficient transition inside the limiter(basically, lookahead like a regular limiter)
- (post_amp) specifies the pre-amp after the limiter before the MPX section
- (limit) specifies the maximum of the sum of amplitudes of all the frequency bins in the limiter.
- (save_harmonics) say if 1khz is really loud and everything else is quiet, setting this to a low value will bring the quiet part up and limit 1khz, setting to a high value will do the opposite. Be careful setting this to a low value.
- (bins) the number of bins to split the frequency range of 1 - 16 khz in to. only values 5,9,15,30,45 are accepted. higher values will give less of a "sound through the tube" effect but will require more processing power.
### AGC section
```
[agc]
attack=0.06
release=0.004
target=6.2e+9
noise_th=4e+6
```
### Bass Boost section
```
[bassboost]
boost=0.4
```
- This controls how much bass to add. Do not add too much bass if your transmitter has attenuated bass; this will make the other parts of the audio sound distorted by the bass.
### EQ section
```
[eq]
0=0.005
1=0.1
2=0.7
3=0.9
4=1
5=1
6=1
7=1.1
8=1.11
```
- For each bin specified in the limiter, you can set its relative presence in the audio.
### Release section
```
[release]
0=0.0000001
1=0.00001
2=0.001
3=0.01
```
- The limiter has a special attack method, but setting the release values to low values can help with frequencies that sound too clipped.



