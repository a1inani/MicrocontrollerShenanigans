Catalogues the process of figuring out how this unique chip works.

I bought two off of eBay, assuming thehy were clones of the Raspberry Pi Pico W. At the time I did not know much about the chip, specifically, what wireless chip it came with. So this looked pretty normal.

![image](https://github.com/user-attachments/assets/44f75e18-558e-405f-bfda-1350edb80838)

While it has an RP2040, the eagle-eyed among you will notice that it makes use of an ESP8285 as its network module in place of the CYW43439. That makes it less of a Raspberry Pi Pico W clone and more of a Raspberry Pi Pico clone + ESP8285. So, why is this an important distinction? While on paper the two boards (this one and a Pico W) would have more or less the same functionality, you would not be able to use the WiFi functionality through the commands you might see in example programs created for the Pico W.

All hope is not lost though. Chips that have some sort of network functionality provide mechanisms through which other chips can communicate with them and generally control their behaviour. 

After a bit of Googling, I found a link (https://mega.nz/folder/1TQ1mCgB#08XOqEZgDEoZkZh1CIOP-A) from a Chinese vendor of similar boards, containing some sample code that I have uploaded in the named folder.
