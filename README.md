# FM/AM broadcast Processor and Audio Maximizer for Edge Devices, specifically for Raspberry Pi.
## 94-100% single core utilization with the provided settings on the Raspberry Pi Zero W 
## For use with the RPI DAC+ module https://www.raspberrypi.com/products/dac-plus/


## Initial Setup
- **Download**: ```git clone https://github.com/radiopushka/VostokRadioLite```
- **Compilation**: ```cd VostokRadioLite``` then ```make```
- **Installation**: ```cp ./vtkradio /usr/bin``` or where ever you want to put it.
- **Configuration**: the sample_cfg file has all the configuration options available, pass it to vtkradio: ```vtkradio ./sample_cfg``` and it will show you the active configuration.
